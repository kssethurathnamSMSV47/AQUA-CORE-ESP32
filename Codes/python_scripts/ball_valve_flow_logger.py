#!/usr/bin/env python3
"""Separate ball-valve opening vs flow logger/report.

Install: python -m pip install pyserial pandas matplotlib python-docx
Run: python ball_valve_flow_logger.py --port COM5
Commands are sent to ESP32: VALVE, 25, 50, 75, 100, STOP, STATUS, REPORT,
HELP, /quit. The operator must physically set the valve opening to the requested
position before confirming it. Do not treat valve opening percentage as pump duty.
"""
from __future__ import annotations
import argparse, csv, datetime as dt, math, queue, re, sys, threading, time
from pathlib import Path
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import serial
from docx import Document
from docx.shared import Inches, Pt

DATA_RE=re.compile(r"^DATA,(\d+),([^,]+),(\d+),([-+0-9.eE]+),([-+0-9.eE]+),([-+0-9.eE]+),([01]),([01]),([01])\s*$")
VALVE_RE=re.compile(r"BALL VALVE POSITION\s*=\s*(25|50|75|100)%",re.I)
FLOW_RE=re.compile(r"Stable flow\s*=\s*([-+0-9.eE]+)\s*L/min",re.I)
COLS=["esp_millis","mode","pulses","frequency_hz","flow_lpm","total_volume_l","float_triggered","excess_event","solenoid_open","host_timestamp"]

def now(): return dt.datetime.now().astimezone().isoformat(timespec="milliseconds")

def main():
    ap=argparse.ArgumentParser(description="Ball-valve experiment logger and report")
    ap.add_argument("--port",required=True); ap.add_argument("--baud",type=int,default=115200); ap.add_argument("--outdir",default="ball_valve_runs")
    ap.add_argument("--a",type=float,default=.0.02420192); ap.add_argument("--b",type=float,default=0.02599480)
    args=ap.parse_args()
    out=Path(args.outdir)/dt.datetime.now().strftime("run_%Y%m%d_%H%M%S"); out.mkdir(parents=True)
    raw_path=out/"serial_log.txt"; samples_path=out/"samples.csv"; results_path=out/"valve_results.csv"
    rows=[]; raw=[]; results=[]; active_pos=None
    print(f"Output folder: {out.resolve()}")
    print("Commands: VALVE starts a valve test; physically set valve to prompted opening; enter 25, 50, 75, 100 when requested.")
    print("Type STOP to stop, REPORT for ESP32 report, /quit to finalize. Close Arduino Serial Monitor first.")
    ser=serial.Serial(args.port,args.baud,timeout=.25); time.sleep(2)
    q=queue.Queue(); stop=threading.Event()
    def keyboard():
        while not stop.is_set():
            line=sys.stdin.readline()
            if not line: break
            line=line.strip()
            if line.lower()=="/quit": stop.set(); break
            if line: q.put(line)
    threading.Thread(target=keyboard,daemon=True).start()
    with raw_path.open("w",encoding="utf-8",buffering=1) as rf, samples_path.open("w",newline="",encoding="utf-8") as sf:
        writer=csv.DictWriter(sf,fieldnames=COLS); writer.writeheader()
        try:
            while not stop.is_set():
                while not q.empty():
                    cmd=q.get(); ser.write((cmd+"\n").encode()); rf.write(f"[HOST {now()}] {cmd}\n"); print(f"[sent] {cmd}")
                b=ser.readline()
                if not b: continue
                line=b.decode("utf-8",errors="replace").strip(); ts=now(); print(line); rf.write(f"[{ts}] {line}\n"); raw.append(line)
                vm=VALVE_RE.search(line)
                if vm: active_pos=int(vm.group(1))
                fm=FLOW_RE.search(line)
                if fm and active_pos is not None:
                    results.append({"opening_percent":active_pos,"stable_flow_lpm":float(fm.group(1)),"source":"ESP32 stable-flow result","host_timestamp":ts}); active_pos=None
                dm=DATA_RE.match(line)
                if dm:
                    row={"esp_millis":int(dm.group(1)),"mode":dm.group(2),"pulses":int(dm.group(3)),"frequency_hz":float(dm.group(4)),"flow_lpm":float(dm.group(5)),"total_volume_l":float(dm.group(6)),"float_triggered":int(dm.group(7)),"excess_event":int(dm.group(8)),"solenoid_open":int(dm.group(9)),"host_timestamp":ts}
                    rows.append(row); writer.writerow(row); sf.flush()
                if "SYSTEM STOP COMPLETE" in line: time.sleep(.5); stop.set()
        except KeyboardInterrupt: print("\nCtrl+C: finalizing.")
        finally: stop.set(); ser.close()
    pd.DataFrame(rows,columns=COLS).to_csv(samples_path,index=False)
    # Preserve actual stable-flow outputs and aggregate repeats by opening.
    r=pd.DataFrame(results,columns=["opening_percent","stable_flow_lpm","source","host_timestamp"])
    r.to_csv(results_path,index=False)
    plots=[]
    if not r.empty:
        summary=r.groupby("opening_percent").stable_flow_lpm.agg(["count","mean","std","min","max"]).reset_index()
        summary.to_csv(out/"valve_summary.csv",index=False)
        fig,ax=plt.subplots(figsize=(8,5))
        ax.errorbar(summary.opening_percent,summary["mean"],yerr=summary["std"].fillna(0),fmt="o-",capsize=4)
        ax.set(title="Ball-Valve Opening vs Measured Flow",xlabel="Valve opening (%)",ylabel="Stable flow (L/min)")
        ax.grid(True,alpha=.3); fig.tight_layout()
        p=out/"ball_valve_opening_vs_flow.png"; fig.savefig(p,dpi=180); plt.close(fig); plots.append(p)
    if not pd.DataFrame(rows).empty:
        d=pd.DataFrame(rows)
        fig,ax=plt.subplots(figsize=(8,5)); ax.scatter(d.frequency_hz,d.flow_lpm,s=16,alpha=.7)
        ax.set(title="FT-01 Flow vs Pulse Frequency During Valve Tests",xlabel="Pulse frequency (Hz)",ylabel="Firmware-calculated flow (L/min)")
        ax.grid(True,alpha=.3); fig.tight_layout(); p=out/"ft01_flow_vs_frequency.png"; fig.savefig(p,dpi=180); plt.close(fig); plots.append(p)
    doc=Document(); sec=doc.sections[0]; sec.top_margin=Inches(.65); sec.bottom_margin=Inches(.65)
    doc.styles["Normal"].font.name="Arial"; doc.styles["Normal"].font.size=Pt(9)
    doc.add_heading("LAM Stage 2 — Ball Valve Characterization Report",0); doc.add_paragraph(f"Generated: {now()}")
    doc.add_paragraph("Valve opening percentage is a manually set ball-valve position, not pump PWM duty cycle. Flow values are FT-01-derived unless independently measured.")
    doc.add_heading("1. Stable flow by valve position",1)
    if r.empty: doc.add_paragraph("No recognised stable-flow valve results were recorded. Check firmware output includes BALL VALVE POSITION and Stable flow lines.")
    else:
        doc.add_paragraph(f"Recognised measurements: {len(r)}; overall mean flow {r.stable_flow_lpm.mean():.3f} L/min; min {r.stable_flow_lpm.min():.3f}; max {r.stable_flow_lpm.max():.3f}.")
        t=doc.add_table(rows=1,cols=6); t.style="Light Shading Accent 1"
        for c,h in zip(t.rows[0].cells,["Opening %","Trials","Mean L/min","SD L/min","Min L/min","Max L/min"]): c.text=h
        for _,row in summary.iterrows():
            cells=t.add_row().cells
            vals=[str(int(row.opening_percent)),str(int(row["count"])),f"{row['mean']:.3f}",f"{row['std']:.3f}" if pd.notna(row["std"]) else "N/A",f"{row['min']:.3f}",f"{row['max']:.3f}"]
            for c,v in zip(cells,vals): c.text=v
    doc.add_heading("2. Captured telemetry",1)
    d=pd.DataFrame(rows)
    if not d.empty: doc.add_paragraph(f"Samples: {len(d)}; mean flow {d.flow_lpm.mean():.3f} L/min; min {d.flow_lpm.min():.3f}; max {d.flow_lpm.max():.3f} L/min; mean frequency {d.frequency_hz.mean():.3f} Hz.")
    else: doc.add_paragraph("No DATA telemetry rows captured.")
    doc.add_heading("3. Data integrity and limitations",1)
    doc.add_paragraph("The FT-01 frequency-to-flow plot is not independent calibration validation because the flow value comes from the same sensor/calibration. For independent calibration, collect a known volume over a timed interval and compare reference flow with frequency.")
    doc.add_paragraph("Repeat each valve position several times after the flow stabilizes. Mean and sample standard deviation are reported for repeated trials. If only one trial exists, SD is undefined.")
    for p in plots: doc.add_paragraph(p.name); doc.add_picture(str(p),width=Inches(6.2))
    report=out/"Ball_Valve_Experiment_Report.docx"; doc.save(report)
    print("\nFINALIZED"); print(f"Raw log: {raw_path}\nSamples: {samples_path}\nValve results: {results_path}\nReport: {report}\nPlots: {len(plots)}")

if __name__=="__main__": main()

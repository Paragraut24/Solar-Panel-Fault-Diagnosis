"""
Utility script to export and convert Firebase Realtime Database telemetry
into standardized CSV format for exploratory analysis and ML training.

Expected CSV Columns:
timestamp,experiment_id,label,coverage_pct,load_state,voltage_v,current_a,power_w,panel_temp_c,g_proxy
"""

import argparse
import csv
import json
import sys
import urllib.request
from pathlib import Path


CSV_HEADERS = [
    "timestamp",
    "experiment_id",
    "label",
    "coverage_pct",
    "load_state",
    "voltage_v",
    "current_a",
    "power_w",
    "panel_temp_c",
    "g_proxy",
]


def parse_telemetry_dict(data):
    """Normalize telemetry dict/list into a flat list of records."""
    records = []
    if not data:
        return records

    # Firebase RTDB returns either a dict keyed by push ID or a list of items
    iterator = data.values() if isinstance(data, dict) else data

    for item in iterator:
        if not isinstance(item, dict):
            continue

        timestamp = item.get("timestamp", "")
        experiment_id = item.get("experiment_id", "EXP-000")
        label = item.get("label", "normal")
        coverage_pct = item.get("coverage_pct", 0.0)
        load_state = item.get("load_state", "resistive")

        voltage_v = float(item.get("voltage_v", 0.0))
        current_a = float(item.get("current_a", 0.0))
        power_w = float(item.get("power_w", voltage_v * current_a))
        panel_temp_c = float(item.get("panel_temp_c", 25.0))
        g_proxy = float(item.get("g_proxy", 0.0))

        records.append({
            "timestamp": timestamp,
            "experiment_id": experiment_id,
            "label": label,
            "coverage_pct": coverage_pct,
            "load_state": load_state,
            "voltage_v": voltage_v,
            "current_a": current_a,
            "power_w": power_w,
            "panel_temp_c": panel_temp_c,
            "g_proxy": g_proxy,
        })

    # Sort records chronologically if timestamp exists
    try:
        records.sort(key=lambda r: int(r["timestamp"]))
    except (ValueError, TypeError):
        pass

    return records


def fetch_from_firebase_url(database_url, path="telemetry"):
    """Fetch raw telemetry JSON from Firebase Realtime Database REST endpoint."""
    clean_url = database_url.rstrip("/")
    endpoint = f"{clean_url}/{path.strip('/')}.json"
    req = urllib.request.Request(
        endpoint,
        headers={"User-Agent": "SolarFaultDiagnosis-Exporter/1.0"}
    )
    with urllib.request.urlopen(req, timeout=30) as response:
        return json.loads(response.read().decode("utf-8"))


def load_from_json_file(file_path):
    """Load telemetry JSON from a local exported file."""
    with open(file_path, "r", encoding="utf-8") as f:
        return json.load(f)


def write_csv(records, output_path):
    """Write parsed records to CSV."""
    out = Path(output_path)
    out.parent.mkdir(parents=True, exist_ok=True)

    with open(out, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=CSV_HEADERS)
        writer.writeheader()
        writer.writerows(records)


def main():
    parser = argparse.ArgumentParser(description="Convert Firebase RTDB JSON telemetry to CSV.")
    parser.add_argument("--input", "-i", type=str, help="Path to downloaded Firebase JSON export file.")
    parser.add_argument("--db-url", "-u", type=str, help="Firebase RTDB base URL (e.g. https://<id>.firebaseio.com).")
    parser.add_argument("--path", "-p", type=str, default="telemetry", help="RTDB node path to fetch (default: telemetry).")
    parser.add_argument("--output", "-o", type=str, required=True, help="Destination CSV file path.")
    parser.add_argument("--filter-exp", type=str, default=None, help="Optional experiment_id filter.")

    args = parser.parse_args()

    if not args.input and not args.db_url:
        print("[ERROR] Please provide either --input <file.json> or --db-url <url>", file=sys.stderr)
        sys.exit(1)

    if args.input:
        print(f"[INFO] Loading JSON from local file: {args.input}")
        raw_data = load_from_json_file(args.input)
    else:
        print(f"[INFO] Fetching telemetry from Firebase endpoint: {args.db_url}")
        raw_data = fetch_from_firebase_url(args.db_url, args.path)

    records = parse_telemetry_dict(raw_data)

    if args.filter_exp:
        records = [r for r in records if r["experiment_id"] == args.filter_exp]
        print(f"[INFO] Filtered to {len(records)} records for experiment '{args.filter_exp}'")

    write_csv(records, args.output)
    print(f"[SUCCESS] Wrote {len(records)} rows to {args.output}")


if __name__ == "__main__":
    main()

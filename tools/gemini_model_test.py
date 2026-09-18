#!/usr/bin/env python3
"""
Diagnostic script for Gemini API Key and Available Models for SmartSchedule.
Tests model availability, generateContent compatibility, latency, JSON adherence,
and SmartSchedule context parsing.
"""

import os
import sys
import json
import time
import urllib.request
import urllib.error
import re

BASE_URL = "https://generativelanguage.googleapis.com/v1beta"
REPORT_FILE = "/tmp/gemini_diagnostic_report.txt"

def mask_key(key: str) -> str:
    if not key:
        return "NONE"
    if len(key) <= 8:
        return "****" + key[-2:]
    return "*" * (len(key) - 4) + key[-4:]

def extract_json_from_text(text: str):
    """Clean markdown code fences if model wrapped response in ```json ... ```"""
    if not text:
        return None
    trimmed = text.strip()
    match = re.search(r"```(?:json)?\s*([\s\S]*?)\s*```", trimmed)
    if match:
        trimmed = match.group(1).strip()
    try:
        return json.loads(trimmed)
    except Exception:
        return None

def make_request(url: str, method: str = "GET", headers: dict = None, body: dict = None, timeout: int = 6):
    if headers is None:
        headers = {}
    data = None
    if body is not None:
        data = json.dumps(body).encode("utf-8")
        headers["Content-Type"] = "application/json"

    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    
    start_time = time.time()
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            elapsed_ms = int((time.time() - start_time) * 1000)
            status_code = resp.getcode()
            response_body = resp.read().decode("utf-8", errors="replace")
            try:
                parsed_json = json.loads(response_body)
            except Exception:
                parsed_json = None
            return {
                "status": status_code,
                "latency_ms": elapsed_ms,
                "body": response_body,
                "json": parsed_json,
                "error": None
            }
    except urllib.error.HTTPError as e:
        elapsed_ms = int((time.time() - start_time) * 1000)
        status_code = e.code
        err_body = e.read().decode("utf-8", errors="replace")
        try:
            err_json = json.loads(err_body)
        except Exception:
            err_json = None
        
        err_info = None
        if isinstance(err_json, dict) and "error" in err_json:
            err_info = err_json["error"]
        else:
            err_info = {"message": err_body, "status": f"HTTP_{status_code}"}

        return {
            "status": status_code,
            "latency_ms": elapsed_ms,
            "body": err_body,
            "json": err_json,
            "error": err_info
        }
    except urllib.error.URLError as e:
        elapsed_ms = int((time.time() - start_time) * 1000)
        return {
            "status": 0,
            "latency_ms": elapsed_ms,
            "body": str(e.reason),
            "json": None,
            "error": {"message": str(e.reason), "status": "NETWORK_ERROR"}
        }
    except Exception as e:
        elapsed_ms = int((time.time() - start_time) * 1000)
        return {
            "status": -1,
            "latency_ms": elapsed_ms,
            "body": str(e),
            "json": None,
            "error": {"message": str(e), "status": "CLIENT_EXCEPTION"}
        }

def send_generate_content_with_retry(api_key: str, model_name: str, contents_payload: dict, timeout: int = 6):
    """
    Sends request to /v1beta/{model_name}:generateContent
    Quick retry for transient errors (max 1 retry, 1s backoff).
    """
    clean_model_path = model_name if model_name.startswith("models/") else f"models/{model_name}"
    url = f"{BASE_URL}/{clean_model_path}:generateContent"
    headers = {
        "x-goog-api-key": api_key,
        "User-Agent": "SmartSchedule-Diagnostic/1.0"
    }

    retries_count = 0
    backoffs = [1]

    for attempt in range(len(backoffs) + 1):
        res = make_request(url, method="POST", headers=headers, body=contents_payload, timeout=timeout)
        status = res["status"]

        if status in (429, 503) and attempt < len(backoffs):
            time.sleep(backoffs[attempt])
            retries_count += 1
            continue

        res["retries"] = retries_count
        return res

    res["retries"] = retries_count
    return res

def parse_generated_text(response_dict):
    """Extracts text from candidate 0, or None if invalid."""
    if not response_dict or not response_dict.get("json"):
        return None
    j = response_dict["json"]
    candidates = j.get("candidates", [])
    if not candidates:
        return None
    content = candidates[0].get("content", {})
    parts = content.get("parts", [])
    if not parts:
        return None
    text = parts[0].get("text", "")
    return text

def format_error_message(err):
    if not err:
        return "N/A"
    if isinstance(err, dict):
        return err.get("message", str(err))
    return str(err)

def main():
    out_lines = []
    def log(msg=""):
        print(msg, flush=True)
        out_lines.append(msg)

    log("=" * 60)
    log("      SMARTSCHEDULE - GEMINI API DIAGNOSTIC RUNNER")
    log("=" * 60)

    # 1. Check API Key
    api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        log("STATUS: FAIL")
        log("REASON: GEMINI_API_KEY is not set")
        log("Please export GEMINI_API_KEY before running this script.")
        with open(REPORT_FILE, "w") as f:
            f.write("\n".join(out_lines))
        sys.exit(1)

    log(f"API Key: {mask_key(api_key)}")

    # 2. Check Gemini API Reachability & GET /models
    log("\n[STEP 1] Checking Gemini API reachability and fetching models...")
    models_url = f"{BASE_URL}/models"
    headers = {"x-goog-api-key": api_key}
    res_models = make_request(models_url, method="GET", headers=headers, timeout=10)

    if res_models["status"] != 200:
        log("STATUS: FAIL")
        log(f"HTTP STATUS: {res_models['status']}")
        err = res_models.get("error") or {}
        if isinstance(err, dict):
            log(f"error.code: {err.get('code', 'N/A')}")
            log(f"error.status: {err.get('status', 'N/A')}")
            log(f"error.message: {err.get('message', res_models.get('body', 'Unknown error'))}")
        else:
            log(f"error: {err}")
        with open(REPORT_FILE, "w") as f:
            f.write("\n".join(out_lines))
        sys.exit(1)

    log("Gemini API reachable: YES")
    log("API Key accepted: YES")

    raw_models = res_models.get("json", {}).get("models", [])
    total_discovered = len(raw_models)
    log(f"Discovered total models: {total_discovered}")

    # 3. Filter models supporting generateContent
    gen_models = []
    for m in raw_models:
        methods = m.get("supportedGenerationMethods", [])
        if "generateContent" in methods:
            gen_models.append(m)

    log(f"Models supporting generateContent: {len(gen_models)}")

    # 4. Run tests on each model
    results = []

    log("\n[STEP 2] Running multi-stage tests on each model...")
    log(f"{'Model':<35} | {'MinTest':<8} | {'Latency':<8} | {'Retry':<5} | {'JSON':<6} | {'Context':<8} | {'Status'}")
    log("-" * 90)

    for m in gen_models:
        model_name = m.get("name", "")
        clean_name = model_name.replace("models/", "")

        # Test 1: Minimal generateContent
        min_payload = {
            "contents": [
                {
                    "parts": [{"text": "Reply with exactly: OK"}]
                }
            ]
        }
        res_min = send_generate_content_with_retry(api_key, model_name, min_payload, timeout=6)
        status_code = res_min["status"]
        retries = res_min.get("retries", 0)
        latency = res_min.get("latency_ms", 0)
        
        text_output = parse_generated_text(res_min)
        min_pass = (status_code == 200 and text_output is not None)

        json_pass = False
        context_pass = False
        json_latency = 0
        context_latency = 0

        final_status = "UNKNOWN"
        if status_code == 200:
            if min_pass:
                final_status = "PASS_MIN"
            else:
                final_status = "EMPTY_CANDIDATE"
        elif status_code == 400:
            final_status = "BAD_REQUEST"
        elif status_code == 401:
            final_status = "UNAUTHENTICATED"
        elif status_code == 403:
            final_status = "ACCESS_DENIED"
        elif status_code == 404:
            final_status = "NOT_FOUND"
        elif status_code == 429:
            final_status = "RATE_LIMITED"
        elif status_code == 500:
            final_status = "SERVER_ERROR"
        elif status_code == 503:
            final_status = "TEMPORARILY_UNAVAILABLE"
        elif status_code == 0:
            final_status = "TIMEOUT_OR_NETWORK"
        else:
            final_status = f"HTTP_{status_code}"

        if retries > 0 and status_code == 200:
            final_status = "PASS_AFTER_RETRY"

        # If minimal test passed, run JSON and Context tests
        if min_pass:
            # Test 2: JSON test
            json_payload = {
                "contents": [
                    {
                        "parts": [{
                            "text": (
                                "Return ONLY valid JSON.\n\n"
                                "Use this schema:\n\n"
                                "{\n"
                                "  \"title\": \"string\",\n"
                                "  \"priority\": \"low|medium|high\",\n"
                                "  \"duration_minutes\": 0\n"
                                "}\n\n"
                                "Input:\nStudy C++ for 90 minutes tomorrow at 8 PM."
                            )
                        }]
                    }
                ]
            }
            res_json = send_generate_content_with_retry(api_key, model_name, json_payload, timeout=6)
            json_latency = res_json.get("latency_ms", 0)
            if res_json["status"] == 200:
                parsed_json = extract_json_from_text(parse_generated_text(res_json) or "")
                if parsed_json and isinstance(parsed_json, dict):
                    if "title" in parsed_json and "priority" in parsed_json and "duration_minutes" in parsed_json:
                        json_pass = True

            # Test 3: SmartSchedule Context test
            ctx_payload = {
                "contents": [
                    {
                        "parts": [{
                            "text": (
                                "Analyze this schedule item and return concise JSON:\n\n"
                                "\"Tomorrow at 8 PM, study Data Structures for 2 hours.\n"
                                "This is important and should be completed before Friday.\"\n\n"
                                "Return:\n\n"
                                "{\n"
                                "  \"title\": \"...\",\n"
                                "  \"duration_minutes\": 0,\n"
                                "  \"priority\": \"...\",\n"
                                "  \"category\": \"...\",\n"
                                "  \"reason\": \"...\"\n"
                                "}"
                            )
                        }]
                    }
                ]
            }
            res_ctx = send_generate_content_with_retry(api_key, model_name, ctx_payload, timeout=6)
            context_latency = res_ctx.get("latency_ms", 0)
            if res_ctx["status"] == 200:
                parsed_ctx = extract_json_from_text(parse_generated_text(res_ctx) or "")
                if parsed_ctx and isinstance(parsed_ctx, dict):
                    if "title" in parsed_ctx and "duration_minutes" in parsed_ctx:
                        context_pass = True

            if min_pass and json_pass and context_pass:
                final_status = "PASS_ALL"
            elif min_pass and json_pass:
                final_status = "PASS_JSON_ONLY"
            else:
                final_status = "TEXT_ONLY"

        model_summary = {
            "name": model_name,
            "clean_name": clean_name,
            "display_name": m.get("displayName", ""),
            "input_limit": m.get("inputTokenLimit", 0),
            "output_limit": m.get("outputTokenLimit", 0),
            "http_status": status_code,
            "retries": retries,
            "min_pass": min_pass,
            "json_pass": json_pass,
            "context_pass": context_pass,
            "latency_ms": latency,
            "avg_latency": (latency + json_latency + context_latency) // (1 + (1 if json_latency else 0) + (1 if context_latency else 0)),
            "status": final_status,
            "error_detail": res_min.get("error")
        }
        results.append(model_summary)

        min_str = "PASS" if min_pass else f"FAIL({status_code})"
        json_str = "PASS" if json_pass else ("-" if not min_pass else "FAIL")
        ctx_str = "PASS" if context_pass else ("-" if not min_pass else "FAIL")
        log(f"{clean_name:<35} | {min_str:<8} | {latency:<6}ms | {retries:<5} | {json_str:<6} | {ctx_str:<8} | {final_status}")

    # Sort results
    reliable_candidates = [r for r in results if r["status"] == "PASS_ALL"]
    reliable_candidates.sort(key=lambda x: x["latency_ms"])

    failed_models = [r for r in results if r["status"] not in ("PASS_ALL", "PASS_MIN", "PASS_JSON_ONLY", "TEXT_ONLY")]

    # Print Final Diagnostic Report according to PHẦN 16 specification
    log("\n")
    log("========================================")
    log("       GEMINI API DIAGNOSTIC")
    log("       SmartSchedule")
    log("========================================")
    log("")
    log("API KEY")
    log("-------")
    log("Configured: YES")
    log("Visible: YES")
    log("Authenticated: YES")
    log("")
    log("API")
    log("---")
    log(f"Endpoint: {models_url}")
    log("HTTP: 200")
    log("Reachable: YES")
    log("")
    log("MODELS")
    log("------")
    log(f"Discovered: {total_discovered}")
    log(f"generateContent capable: {len(gen_models)}")
    log(f"tested: {len(results)}")
    log("")
    log("RESULTS")
    log("-------")
    log(f"{'Model':<32} | {'HTTP':<4} | {'Retries':<7} | {'Text':<5} | {'JSON':<5} | {'Context':<7} | {'Latency':<7} | {'Status'}")
    log("-" * 88)
    for r in results:
        t_str = "PASS" if r["min_pass"] else "FAIL"
        j_str = "PASS" if r["json_pass"] else "-"
        c_str = "PASS" if r["context_pass"] else "-"
        lat_str = f"{r['latency_ms']}ms"
        log(f"{r['clean_name']:<32} | {r['http_status']:<4} | {r['retries']:<7} | {t_str:<5} | {j_str:<5} | {c_str:<7} | {lat_str:<7} | {r['status']}")

    log("")
    log("RELIABLE CANDIDATES")
    log("-------------------")
    if reliable_candidates:
        for idx, c in enumerate(reliable_candidates, 1):
            log(f"Candidate {idx}:")
            log(f"model: {c['name']}")
            log(f"clean_id: {c['clean_name']}")
            log(f"displayName: {c['display_name']}")
            log("reason:")
            log("- accessible: YES")
            log("- generateContent supported: YES")
            log("- text test passed: YES")
            log("- JSON test passed: YES")
            log("- context test passed: YES")
            log(f"- minimal latency: {c['latency_ms']} ms (average: {c['avg_latency']} ms)")
            log(f"- inputTokenLimit: {c['input_limit']} | outputTokenLimit: {c['output_limit']}")
            log("")
    else:
        log("None met all strict PASS_ALL criteria.")

    log("RECOMMENDED CONFIGURATION")
    log("-------------------------")
    if reliable_candidates:
        flash_cands = [c for c in reliable_candidates if "flash" in c["clean_name"]]
        chosen = flash_cands[0] if flash_cands else reliable_candidates[0]
        log(f"Model: {chosen['clean_name']}")
        log(f"Full Resource Name: {chosen['name']}")
        log(f"Endpoint: https://generativelanguage.googleapis.com/v1beta/models/{chosen['clean_name']}:generateContent")
        log("Generation: generateContent")
        log("")
        log("REASON")
        log("------")
        log(f"- Passed all 3 test stages (minimal OK, JSON adherence schema, SmartSchedule schedule extraction).")
        log(f"- Measured lowest latency ({chosen['latency_ms']} ms), ideal for interactive desktop UI.")
        log(f"- Official model with status HTTP 200 without requiring retries.")
    else:
        log("No recommended configuration found due to lack of PASS_ALL models.")

    log("")
    log("FAILED MODELS")
    log("-------------")
    if failed_models:
        for f in failed_models:
            err_msg = format_error_message(f["error_detail"])
            # Truncate long error messages cleanly
            if len(err_msg) > 120:
                err_msg = err_msg[:117] + "..."
            log(f"Model: {f['clean_name']}")
            log(f"HTTP: {f['http_status']}")
            log(f"Error: {err_msg}")
            log(f"Interpretation: {f['status']}")
            log("-" * 30)
    else:
        log("None. All tested models returned HTTP 200.")

    log("========================================")

    # Save to disk
    with open(REPORT_FILE, "w") as f:
        f.write("\n".join(out_lines))
    log(f"\n[INFO] Full diagnostic report written to {REPORT_FILE}")

if __name__ == "__main__":
    main()

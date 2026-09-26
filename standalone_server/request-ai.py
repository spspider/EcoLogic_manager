from dotenv import load_dotenv
import os
import requests
import json

# Load environment variables
load_dotenv("../.env")
api_key = os.getenv("EPAM_DIAL_KEY")

if not api_key:
    raise ValueError("EPAM_DIAL_KEY not found in environment variables")

# ---------------------------------------------------------
# 1. Fetch all models available to your key
# ---------------------------------------------------------
models_url = "https://ai-proxy.lab.epam.com/openai/models"
models = requests.get(models_url, headers={"Api-Key": api_key}).json()["data"]

# Extract only model IDs
model_ids = [m["id"] for m in models]

print("Available models:")
for m in model_ids:
    print(" -", m)

# ---------------------------------------------------------
# 2. Choose a model automatically
#    Option A: first available model
#    Option B: filter by name (e.g., Claude)
# ---------------------------------------------------------

# Example: choose Claude Sonnet automatically if present
preferred = None
for m in model_ids:
    if "claude-sonnet" in m:
        preferred = m
        break

# Fallback: first model
if preferred is None:
    preferred = model_ids[0]

print(f"\nSelected model: {preferred}")

# ---------------------------------------------------------
# 3. Build the chat completion URL dynamically
# ---------------------------------------------------------
chat_url = f"https://ai-proxy.lab.epam.com/openai/deployments/{preferred}/chat/completions"

payload = {
    "messages": [{"role": "user", "content": "Hello from dynamic model selection!"}]
}

response = requests.post(
    chat_url,
    headers={
        "Content-Type": "application/json",
        "Api-Key": api_key
    },
    json=payload
)

print("\nStatus:", response.status_code)
print("Response:")
print(json.dumps(response.json(), indent=4))

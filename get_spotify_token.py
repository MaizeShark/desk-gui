"""
A one-stop script to get a Spotify Refresh Token.

This script will:
1. Start a temporary local web server.
2. Open your browser to the Spotify authorization page.
3. After you log in and grant permission, it will catch the redirect.
4. It will then automatically exchange the received authorization code
   for a permanent Refresh Token.
5. Finally, it will print the Refresh Token for you to copy.

You might need to install the 'requests' library first:
pip install requests
"""
print("You might need to install the 'requests' library first: pip install requests")

import http.server
import socketserver
import webbrowser
import base64
import requests
from urllib.parse import urlparse, parse_qs, urlencode

# --- STEP 1: PASTE YOUR SPOTIFY APP CREDENTIALS HERE ---
# You can get these from your Spotify Developer Dashboard:
# https://developer.spotify.com/dashboard/
CLIENT_ID = "7270af283ac647ed8ba230b5826f7d1b"
CLIENT_SECRET = "208eae8bce8b43c1b7a6ea0766d6151e"

# --- Configuration (usually no need to change) ---
PORT = 8888
REDIRECT_URI = f"http://127.0.0.1:{PORT}/callback"

# The permissions your app is asking for.
# This should match the scopes your ESP32 project needs.
SCOPE = "ugc-image-upload playlist-read-collaborative playlist-modify-private playlist-modify-public playlist-read-private user-read-playback-position user-read-recently-played user-top-read user-modify-playback-state user-read-currently-playing user-read-playback-state user-read-private user-read-email user-library-modify user-library-read user-follow-modify user-follow-read streaming app-remote-control"

# Global variable to hold the refresh token
refresh_token = None

def exchange_code_for_token(auth_code):
    """Exchanges the authorization code for an access and refresh token."""
    global refresh_token

    token_url = "https://accounts.spotify.com/api/token"

    # Base64 encode the client_id:client_secret string
    auth_string = f"{CLIENT_ID}:{CLIENT_SECRET}"
    auth_bytes = auth_string.encode('utf-8')
    auth_base64 = base64.b64encode(auth_bytes).decode('utf-8')

    headers = {
        'Authorization': f"Basic {auth_base64}",
        'Content-Type': 'application/x-www-form-urlencoded'
    }

    data = {
        'grant_type': 'authorization_code',
        'code': auth_code,
        'redirect_uri': REDIRECT_URI
    }

    try:
        response = requests.post(token_url, headers=headers, data=data)
        response.raise_for_status()  # Raises an exception for bad status codes (4xx or 5xx)
        token_data = response.json()

        if 'refresh_token' in token_data:
            refresh_token = token_data['refresh_token']
            return True
        else:
            print("❌ ERROR: 'refresh_token' not found in Spotify's response.")
            print("Full response:", token_data)
            return False

    except requests.exceptions.RequestException as e:
        print(f"❌ ERROR during token exchange: {e}")
        print("Response content:", e.response.text if e.response else "No response")
        return False

class MyHttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        """Handles the GET request and the redirect from Spotify."""
        query_components = parse_qs(urlparse(self.path).query)

        if 'code' in query_components:
            auth_code = query_components["code"][0]

            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b"<html><head><title>Success!</title></head>")
            self.wfile.write(b"<body style='font-family: sans-serif; text-align: center; background-color: #1DB954; color: white;'>")
            self.wfile.write(b"<h1>Authentication Successful!</h1>")
            self.wfile.write(b"<p>Please return to your terminal to get the refresh token.</p>")
            self.wfile.write(b"</body></html>")

            # Perform the token exchange
            if exchange_code_for_token(auth_code):
                # If successful, this will allow the server to shut down
                raise KeyboardInterrupt

        elif 'error' in query_components:
            error = query_components["error"][0]
            print(f"❌ Spotify returned an error: {error}")
            self.send_response(400)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(f"<h1>Error: {error}</h1>".encode('utf-8'))
            raise KeyboardInterrupt


def run_server():
    """Constructs the auth URL and starts the local server."""
    auth_params = {
        'client_id': CLIENT_ID,
        'response_type': 'code',
        'redirect_uri': REDIRECT_URI,
        'scope': SCOPE,
    }
    auth_url = f"https://accounts.spotify.com/authorize?{urlencode(auth_params)}"

    print("=====================================================================")
    print("              Spotify Refresh Token Generator")
    print("=====================================================================")
    print("\nSTEP 2: Your browser will now open to authorize this script.")
    print("If it doesn't, please copy and paste this URL into your browser:")
    print(f"\n{auth_url}\n")

    webbrowser.open(auth_url)

    with socketserver.TCPServer(("", PORT), MyHttpRequestHandler) as httpd:
        print(f"Waiting for Spotify to redirect to http://127.0.0.1:{PORT}/callback ...")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            httpd.server_close()
            print("\nServer has shut down.")

if __name__ == '__main__':
    if CLIENT_ID == "your_spotify_client_id" or CLIENT_SECRET == "your_spotify_client_secret":
        print("❌ ERROR: Please edit the script and paste your Spotify Client ID and")
        print("         Client Secret into the variables (CLIENT_ID and CLIENT_SECRET) at the top of the file.")
    else:
        print("\nSTEP 1: Make sure you've added the following Redirect URI to your")
        print("        app settings in the Spotify Developer Dashboard:")
        print(f"        {REDIRECT_URI}\n")
        run_server()

        if refresh_token:
            print("\n=====================================================================")
            print("✅ SUCCESS! Your Spotify Refresh Token is ready.")
            print("\nCOPY THE LINE BELOW and paste it into your ESP32 code (e.g., Config.h):")
            print("---------------------------------------------------------------------")
            print(f"{refresh_token}")
            print("---------------------------------------------------------------------")
        else:
            print("\nProcess finished, but no refresh token was obtained. Please check for errors above.")

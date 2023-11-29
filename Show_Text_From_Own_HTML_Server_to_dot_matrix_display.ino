#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LedControl.h>

const char *ssid = "ESP8266-AP"; // Set the SSID for the ESP8266 access point
const char *password = "12345678"; // Set the password for the access point

// Define the pin configuration for the MAX7219 module
const int dataPin = D7;  // Connect to the DIN pin of MAX7219
const int clockPin = D5; // Connect to the CLK pin of MAX7219
const int csPin = D8;    // Connect to the CS pin of MAX7219

LedControl lc = LedControl(dataPin, clockPin, csPin, 1); // Create an instance of the LedControl library

ESP8266WebServer server(80);

String receivedData = "";

const char *htmlPage = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Control Panel</title>
        <style>
        * {
            margin: 0;
            padding: 0;
            outline: none;
            box-sizing: border-box;
            font-family: 'Poppins', sans-serif;
        }

        body {
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 10px;
            font-family: 'Poppins', sans-serif;
            background: linear-gradient(115deg, #56d8e4 10%, #9f01ea 90%);
        }

        .container {
            max-width: 800px;
            background: #fff;
            width: 800px;
            padding: 25px 40px 10px 40px;
            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
        }

        .container .text {
            text-align: center;
            font-size: 41px;
            font-weight: 600;
            font-family: 'Poppins', sans-serif;
            background: -webkit-linear-gradient(right, #56d8e4, #9f01ea, #56d8e4, #9f01ea);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .container form {
            padding: 30px 0 0 0;
        }

        .container form .form-row {
            display: flex;
            margin: 32px 0;
        }

        form .form-row .input-data {
            width: 100%;
            height: 40px;
            margin: 0 20px;
            position: relative;
        }

        form .form-row .textarea {
            height: 70px;
        }

        .input-data input,
        .textarea textarea {
            display: block;
            width: 100%;
            height: 100%;
            border: none;
            font-size: 17px;
            border-bottom: 2px solid rgba(0, 0, 0, 0.12);
        }

        .input-data input:focus~label,
        .textarea textarea:focus~label,
        .input-data input:valid~label,
        .textarea textarea:valid~label {
            transform: translateY(-20px);
            font-size: 14px;
            color: #3498db;
        }

        .textarea textarea {
            resize: none;
            padding-top: 10px;
        }

        .input-data label {
            position: absolute;
            pointer-events: none;
            bottom: 10px;
            font-size: 16px;
            transition: all 0.3s ease;
        }

        .textarea label {
            width: 100%;
            bottom: 40px;
            background: #fff;
        }

        .input-data .underline {
            position: absolute;
            bottom: 0;
            height: 2px;
            width: 100%;
        }

        .input-data .underline:before {
            position: absolute;
            content: "";
            height: 2px;
            width: 100%;
            background: #3498db;
            transform: scaleX(0);
            transform-origin: center;
            transition: transform 0.3s ease;
        }

        .input-data input:focus~.underline:before,
        .input-data input:valid~.underline:before,
        .textarea textarea:focus~.underline:before,
        .textarea textarea:valid~.underline:before {
            transform: scale(1);
        }

        .submit-btn .input-data {
            overflow: hidden;
            height: 45px!important;
            width: 25%!important;
        }

        .submit-btn .input-data .inner {
            height: 100%;
            width: 300%;
            position: absolute;
            left: -100%;
            background: -webkit-linear-gradient(right, #56d8e4, #9f01ea, #56d8e4, #9f01ea);
            transition: all 0.4s;
        }

        .submit-btn .input-data:hover .inner {
            left: 0;
        }

        .submit-btn .input-data input {
            background: none;
            border: none;
            color: #fff;
            font-size: 17px;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 1px;
            cursor: pointer;
            position: relative;
            z-index: 2;
        }

        @media (max-width: 700px) {
            .container .text {
                font-size: 30px;
            }

            .container form {
                padding: 10px 0 0 0;
            }

            .container form .form-row {
                display: block;
            }

            form .form-row .input-data {
                margin: 35px 0!important;
            }

            .submit-btn .input-data {
                width: 40%!important;
            }
        }
    </style>
</head>

<body>
    <div class="container">
        <div class="text">ESP8266 Control Panel</div>
        <form action="/set_data" method="get">
            <div class="form-row">
                <div class="input-data textarea">
                    <textarea rows="8" cols="80" name="data" required></textarea>
                    <br />
                    <div class="underline"></div>
                    <label for="">Write your message</label>
                    <br />
                    <div class="form-row submit-btn">
                        <div class="input-data">
                            <div class="inner"></div>
                            <input type="submit" value="submit">
                        </div>
                    </div>
                </div>
            </div>
        </form>
    </div>
</body>

</html>
)";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSetData() {
  receivedData = server.arg("data");
  server.send(200, "text/plain", "Data received: " + receivedData);

  // Display the received data on the 8x32 dot matrix display
  displayOnMatrix(receivedData);

  // Redirect back to the root page
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  // Set up ESP8266 in Access Point (AP) mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Print the IP address of the ESP8266 access point
  Serial.println("ESP8266 Access Point IP Address: " + WiFi.softAPIP().toString());

  // Initialize the MAX7219 display
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  // Define the routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set_data", HTTP_GET, handleSetData);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}

void displayOnMatrix(String message) {
  // Clear the display
  lc.clearDisplay(0);

  // Display the message on the matrix
  for (int i = 0; i < message.length(); i++) {
    char character = message.charAt(i);
    int column = character - ' '; // Adjust for ASCII values
    lc.setChar(0, i, column, false);
  }

  delay(3000); // Display for 3 seconds, adjust as needed

  // Clear the display again
  lc.clearDisplay(0);
}

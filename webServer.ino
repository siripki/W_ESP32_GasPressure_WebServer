void serverSetup() {
  // Serve Static File on Assets
  server.serveStatic("/assets/", SPIFFS, "/assets/");

  // Default Handler 404
  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(200, "text/html", "Destination NOT FOUND !");
  });

  // Handle User Authentication
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->authenticate("admin", "admin")) {
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
    else {
      return request->requestAuthentication();
    }
  });

  // // execute : Handle post request
  // server.on("/execute", HTTP_POST, [](AsyncWebServerRequest* request) {
  //   if (request->authenticate("admin", "admin")) {
  //     if (request->hasParam("mode", true)) {
  //       mode = request->getParam("mode", true)->value();
  //       writeFile("/mode.txt", mode.c_str());
  //     }

  //     request->redirect("/");
  //     Serial.println("Execute OK!");
  //   }

  //   else {
  //     delay(50);
  //     request->send(200, "text/plain", "You don't have permission!");
  //     Serial.println("Execute ERROR !");
  //   }
  // });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient* client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  AsyncElegantOTA.begin(&server);
  server.begin();
}

void serverSentEvents() {
  events.send(String(gasPressure).c_str(), "gasPressure", millis());
}

String processor(const String& var) {
  if (var == "gasPressure") {
    return String(gasPressure,1);
  }

  else {
    return String();
  }
}
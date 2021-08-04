// sendCommand
// Sends the supplied command to the TeamSpeak 3 server and returns the result. If the command does not return a line of information
// or has an error then it returns the error line (the server returns "error id=0 msg=ok" if the command was successful!)
// The function returns "error msg=timeout" if the server stops responding.
String sendCommand(String command)
{
  int timeout;
  String response, line;

  telnet.println(command);

  timeout = millis();
  while (telnet.available() == 0)
  {
    updateDisplay();
    if (millis() - timeout > 1500)
      return "error msg=timeout";
  }

  while (telnet.available())
  {
    line = telnet.readStringUntil('\r');
    line.remove(line.indexOf('\n'));
    if (line.substring(0,5) != "error")
    {
      response = line;
      line = telnet.readStringUntil('\r');
      line.remove(line.indexOf('\n'));
    }
    if (line.substring(0,5) == "error" && line != okResponse)
      response = line;
    else if (line == okResponse && response == "")
      response = line;
  }

  // Wait for a quarter second to make sure there is no more information
  timeout = millis();
  while (telnet.available() == 0)
  {
    updateDisplay();
    if (millis() - timeout > 250)
      break;
  }
  return response;
}

String extractData(String source, String field)
{
  String data = "";
  
  int index = source.indexOf(field);
  if (index == -1)
    return "";

  int index2 = source.indexOf("=", index) + 1;
  if (index2 == 0)
    return "";

  for (int i=index2; i<source.length(); i++)
  {
    if (source.charAt(i) == '\\' && source.charAt(i+1) == 's')
    {
      data += " ";
      i++;
    }
    else if ((source.charAt(i) == ' ') || (source.charAt(i) == '|'))
      break;
    else
      data += source.charAt(i);
  }
  return data;
}

void loginServerQuery()
{
  loginOK = false;

  // Connect to the TeamSpeak query server
  sprintln("Connecting to TeamSpeak query server...");

  if (!telnet.connect(telnetHost, telnetPort))
  {
    sprintln("Connection failed!");
    return;
  }

  // Now wait for the welcome message (Basically read all incomming lines)
  int timeout = millis();
  while (telnet.available() == 0)
  {
    updateDisplay();
    if (millis() - timeout > 1500)
    {
      sprintln(">>> Telnet timeout!");
      telnet.stop();
      return;
    }
  }
  while (telnet.available())
  {
    String line = telnet.readStringUntil('\r');
    line.remove(line.indexOf('\r'));
    sprintln(line);
  }
  timeout = millis();
  while (telnet.available() == 0)
  {
    updateDisplay();
    if (millis() - timeout > 500)
      break;
  }

  // Send the login command
  sprintln("Log in as server admin");
  String command = "login ";
  command += queryLogin;
  command += " ";
  command += queryPass;
  String response = sendCommand(command);
  if (response != okResponse)
  {
    sprint("Error returned: ");
    sprintln(response);
    telnet.stop();
    return;
  }
  else
    sprintln("OK");

  // Send the command to use virtual server 1 (the only server on free servers)
  sprintln("Use server 1");
  response = sendCommand("use 1");
  if (response != okResponse)
  {
    sprint("Error returned: ");
    sprintln(response);
    telnet.stop();
    return;
  }
  else
    sprintln("OK");

  // Send the serverinfo command to get the server's name
  sprintln("Retrieve server info");
  response = sendCommand("serverinfo");
  if (response.substring(0,5) == "error")
  {
    sprint("Error returned: ");
    sprintln(response);
    telnet.stop();
    return;
  }
  else
  {
    sprintln(response); // For now just send the response to the serial connection
    ts3ServerName = extractData(response, "virtualserver_name");
    sprint("Server name:");
    sprintln(ts3ServerName);
  }

  loginOK = true;
}

void processChannels(int pid)
{
  for (int ch = 0; ch < numChannels; ch++)
  {
    if (channels[ch].parentID == pid)
    {
      if (channels[ch].channelName.charAt(0) == '[' && channels[ch].channelName.indexOf("pacer") != -1)
      {
        content += "<hr style='width: 30%; height: 2px; margin-left: 0px; margin-right: auto;'>\n";
      }
      else 
      {
        content += "<big><span style='font-weight: bold;'>";
        content += channels[ch].channelName;
        content +="</span></big><br>\n";
        
        content += "<div style='margin-left: 20px;'>\n";
        for (int i=0; i<numClients; i++)
        {
          if (clients[i].channelID == channels[ch].ID)
          {
            content += clients[i].clientName;
            content += "<br>\n";
          }
        }
        processChannels(channels[ch].ID);
        content += "</div>\n";
      }
    }
  }
}

// Ask the TS3 server for a list of channels (interval defined in CHANNEL_REFRESH)
void refreshChannels()
{
  String response;
  if (millis() - timeoutChan > CHANNEL_REFRESH && loginOK)
  {
    // Send the serverinfo command to get the server's name
    sprintln("Retrieve server info");

    response = sendCommand("serverinfo");
    if (response.substring(0,5) == "error")
    {
      sprint("Error returned: ");
      sprintln(response);

      errorCount++;
    }
    else
    {
      sprintln(response); // For now just send the response to the serial connection

      ts3ServerName = extractData(response, "virtualserver_name");

      sprint("Server name:");
      sprintln(ts3ServerName);
    }
    
    // Get channel list from TeamSpeak
    sprintln("Get channel list");

    response = sendCommand("channellist");
    if (response.substring(0,5) == "error")
    {
      sprint("Error returned: ");
      sprintln(response);
      errorCount++;
    }
    else
    {
      // Process client list returned in response
//      Serial.println(response); // For now just send the response to the serial connection
      int i=0, doLoop=true;
      numChannels = 0;

      String channelData;

      while(doLoop)
      {
        updateDisplay();
        channelData = "";
        for (; i<response.length(); i++)
        {
          if (response.charAt(i) == '|')
          {
            i++;
            break;
          }
          else
            channelData += response.charAt(i);
        }
        String type = extractData(channelData, "client_type");
        String tmp = extractData(channelData, "cid");
        channels[numChannels].ID = tmp.toInt();
        tmp = extractData(channelData, "pid");
        channels[numChannels].parentID = tmp.toInt();
        tmp = extractData(channelData, "channel_order");
        channels[numChannels].channelAboveID = tmp.toInt();
        channels[numChannels].channelName = extractData(channelData, "channel_name");

        numChannels++;
        if (i == response.length() || numChannels == SERVER_MAX_CHANNELS)
          doLoop = false;
      }
    }
    timeoutChan = millis();
  }
}

// Ask the TS3 server for a list of clients (interval defined in CHANNEL_REFRESH)
void refreshClients()
{
  String response;
  if (millis() - timeoutClient > CLIENT_REFRESH)
  {
    sprint("errorCount = ");
    sprintln(String(errorCount));

    if (loginOK)
    {
      // Get client list from TeamSpeak
      sprintln("Get client list");

      response = sendCommand("clientlist");
      if (response.substring(0,5) == "error")
      {
        sprint("Error returned: ");
        sprintln(response);
        errorCount++;
      }
      else
      {
        // Process client list returned in response
//        sprintln(response); // For now just send the response to the serial connection
        int i=0, doLoop=true;
        numClients = 0;
        clientCount = 0;
        errorCount = 0;
  
        String clientData;
  
        while(doLoop)
        {
          updateDisplay();
          clientData = "";
          for (; i<response.length(); i++)
          {
            if (response.charAt(i) == '|')
            {
              i++;
              break;
            }
            else
              clientData += response.charAt(i);
          }
          String type = extractData(clientData, "client_type");
          if (type == "0")
          {
            String tmp = extractData(clientData, "clid");
            clients[numClients].ID = tmp.toInt();
            tmp = extractData(clientData, "cid");
            clients[numClients].channelID = tmp.toInt();
            clients[numClients].clientName = extractData(clientData, "client_nickname");
            if (clients[numClients].clientName != "Galbi3000" && clients[numClients].clientName != "MusicSource" && clients[numClients].clientName != "GalbiOnTab" && clients[numClients].clientName != "GalbiOnPhone")
              clientCount++;
            numClients++;
          }
          if (i == response.length())
            doLoop = false;
        }
        sprint("numClients = ");
        sprintln(String(numClients));
        sprint("clientCount = ");
        sprintln(String(clientCount));
        sprintln("");
      }
    }
    else
    {
      errorCount++;
    }
    timeoutClient = millis();
  }
}

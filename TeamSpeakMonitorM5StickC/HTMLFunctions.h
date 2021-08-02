// Root (default) page handling
void handleRoot()
{
  sprintln("Enter handleRoot");
  content = "<html>\n";
  content += "<head>\n";
  content += "<meta content='text/html; charset=ISO-8859-1' http-equiv='content-type'>\n";
  content += "<meta http-equiv='refresh' content='5'>\n<title>";
  content += ts3ServerName;
  content += "</title>\n</head>";
  content += "<body>\n<h2><span style='font-weight: bold;'>";
  content += ts3ServerName;
  content += "</span></h2>\n";

  content += "<div style='margin-left: 20px;'>\n";
  processChannels(0);
  
  content += "</div>\n</body>\n</html>";

  server.send(200, "text/html", content);
}

// Page Not Found handler
void handleNotFound()
{
  content = "Page Not Found\n\n";
  content += "URI: ";
  content += server.uri();
  content += "\nMethod: ";
  content += (server.method() == HTTP_GET)?"GET":"POST";
  content += "\nArguments: ";
  content += server.args();
  content += "\n";
  for (uint8_t i=0; i<server.args(); i++)
  {
    content += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", content);
}


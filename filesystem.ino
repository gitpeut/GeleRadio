#include <FS.h>
#include <FFat.h>
#include <SPIFFS.h>
#include <LITTLEFS.h>
#include <esp_littlefs.h>

//holds the current upload
fs::File fsUploadFile;

//----------------------------------------------

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "b";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "Kb";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "Mb";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "Gb";
  }
}
//----------------------------------------------

String getContentType(String filename) {

  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".json")) return "application/json";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".svg")) return "image/svg+xml";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if(filename.endsWith(".bin")) return "application/octet-stream";
  
  return "text/plain";
}

//----------------------------------------------

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (RadioFS.exists(pathWithGz) || RadioFS.exists(path)) {
    if (RadioFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = RadioFS.open(path, "r");
    server.streamFile(file, contentType);
    
    file.close();
    return true;
  }
  return false;
}

//----------------------------------------------

void handleFileUpload(){

   HTTPUpload& upload = server.upload();
  
   String filename=upload.filename;
   
   if(upload.status == UPLOAD_FILE_START){
        
    Serial.print("Uploading ");Serial.println(filename);
    
    if ( server.hasArg("filename") ){
      filename = server.arg("filename");
      Serial.print("filename in text "); Serial.println(server.arg("filename") );
    }
    
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);

    //reading_file++;
    fsUploadFile = RadioFS.open(filename, "w");

    if ( !fsUploadFile  ){
      Serial.print("Couldn't open ");Serial.println( filename );
    }
    
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
      delay(1);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();

      if ( filename.equals("/stations.json") ){
              int changestation=-1;
            
              read_stations();
                
              if ( server.hasArg("changestation") ){
                 changestation =  server.arg("changestation").toInt(); 
                 Serial.printf("Server has arg changestation: %d\n", changestation );
              }
 
              if ( changestation != -1  ){                          
                  Serial.printf("Changing stations to %d\n", changestation );
                  //xSemaphoreTake( updateSemaphore, portMAX_DELAY);
                     
                     setStation( changestation, changestation );
                    
                     
                  //xSemaphoreGive( updateSemaphore);
   
              }
      }
      //if ( reading_file ) reading_file--;
      
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

//------------------------------------------------

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!RadioFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  RadioFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

//----------------------------------------------

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (RadioFS.exists(path)) {
    return server.send(500, "text/plain", "FILE exists");
  }
  File file = RadioFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

//----------------------------------------------

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = RadioFS.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.name()).substring(1) + "\"";
          if ( !file.isDirectory()){
            output += ", \"size\": ";
            output += String(file.size());
            
          }
          output += "}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "application/json", output);
  
}

//----------------------------------------------
void setupFS(void) {

      switch ( RadioFSNO ){
        case FSNO_SPIFFS:
            SPIFFS.begin();
            break;
        case FSNO_LITTLEFS:
            LITTLEFS.begin();
            break;
        case FSNO_FFAT:    
            FFat.begin();
            break;
      }
     
      File root = RadioFS.open("/");
      File file = root.openNextFile();
      while(file){
          String fileName = file.name();
          size_t fileSize = file.size();
          Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
          if ( ! strcmp(  fileName.c_str(), "/syslog.txt" ) && fileSize > 20000 ){
              RadioFS.remove(fileName);
              Serial.printf("**************removed syslog.txt as it was over 20k\n" );
          }
          file = root.openNextFile();
      }
      Serial.printf("\n");
  
}

//-----------------------------------------------------

int syslog( char *message){
FILE *log=NULL;
time_t now;
now = time(nullptr);
char  tijd[32];
char  filename[128];

sprintf( filename, "%s/syslog.txt", RadioMount);
log = fopen( filename, "a");

if ( log == NULL) {
  Serial.printf("Couldn't open /syslog.txt (errno %d)\n", errno );
  return(-1);
}

sprintf( tijd,"%s", asctime( localtime(&now)) );
tijd[ strlen(tijd) - 1 ] = 0;

fprintf( log,"%s - %s\n", tijd, message);
fclose(log);
return(0);
}

     

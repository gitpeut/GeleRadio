
#define GETBANDFREQ 50 // call getbands after every GETBANDFREQ chunks
#define SKIPSTART 350

void play ( void *param ){
uint8_t   playBuffer[32];
uint32_t  bandcounter=GETBANDFREQ, VSlow=0, skipstartsound=SKIPSTART;

  Serial.printf("Playtask running on core %d\n", xPortGetCoreID()); 

  vs1053player->startSong();

//    vs1053player->getBands() ;
//    displaySpectrum();

  while ( uxQueueMessagesWaiting(playQueue) <  (PLAYQUEUESIZE/2) ) {
    Serial.printf ( "Waiting for Queue to fill up, %d messages in playQueue\n", uxQueueMessagesWaiting( playQueue ) );
    delay(40);
  }
  Serial.printf ( "Queueu filled up, %d messages in playQueue\n", uxQueueMessagesWaiting( playQueue ) );
  
    while(1){
      
      xQueueReceive(playQueue, &playBuffer[0], portMAX_DELAY);

      if ( strncmp( (char *) &playBuffer[0], "ChangeStationSoStartANewSongNow!",32) == 0 ){
        
        skipstartsound = SKIPSTART;
        vs1053player->stopSong();
        delay(5);
        vs1053player->startSong();
        
      }
      
      
        for ( int i = 0; i < 1 ; ++i ){
          if ( digitalRead( vs_dreq_pin ) ){
            //xSemaphoreTake( tftSemaphore, portMAX_DELAY);
            vs1053player->playChunk(playBuffer, 32  );
            //xSemaphoreGive( tftSemaphore);
            
            --bandcounter;
            if ( ! bandcounter ){

              if ( currDisplayScreen == HOME ){
                vs1053player->getBands();
                delay(10); // please the task watchdog
                if ( xSemaphoreGetMutexHolder( updateSemaphore ) == NULL ){
                  xSemaphoreTake( tftSemaphore, portMAX_DELAY);
                  vs1053player->displaySpectrum();
                  xSemaphoreGive( tftSemaphore);
                }
              }
    
              delay(10); // please the task watchdog
              bandcounter = GETBANDFREQ;         
            }

            if ( skipstartsound ){
                if ( skipstartsound <= (getVolume()*4) ){ 
                  vs1053player->setVolume( (getVolume() - (skipstartsound/4) + 1) );                                  
                }
                if ( skipstartsound == 1 ) {
                  //Serial.println("** end fade in **");
                  vs1053player->setVolume( getVolume());
                }
                --skipstartsound;
                
            }

            VSlow = 0;   
          }else{
            --i;  
            VSlow++;
            if ( VSlow > 60 ){         
              if ( VSlow %10 == 0 ){
                Serial.printf ( "Waiting for VS1053, VSlow %d , %d messages in playQueue\n", VSlow, uxQueueMessagesWaiting( playQueue ) );
                syslog( (char *)"VSlow for a long time.");
              }
            }
            delay(3);
          }
      }
     
    }


}


    /*--------------------------------------------------*/

int play_init(){
     
    xTaskCreatePinnedToCore( 
         play,                                      // Task to handle special functions.
         "Player",                                            // name of task.
         2048,                                                 // Stack size of task
         NULL,                                                 // parameter of the task
         PLAYTASKPRIO,                                                    // priority of the task
         &playTask,                                          // Task handle to keep track of created task 
         PLAYCORE);                                                  // processor core
                
return(0);
}

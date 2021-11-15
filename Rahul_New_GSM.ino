char data[40];

void flush_rx_buffer(void)  /* Flush out the data in the receive buffer*/
{
  char c;
  while(Serial.available()!=0)
  {
    c = Serial.read();
  }
}

char UnreadSMS(void)
{
  int8_t i=0;
  char *s;
  char rx_buff1[100];  
  memset(rx_buff1,0,100);
  flush_rx_buffer();
  Serial.println("AT+CMGF=1");  /* Select message format as text */
  delay(700);
  flush_rx_buffer();
  Serial.println("AT+CMGL=\"REC UNREAD\",1"); /* List received messages without changing their status */
  delay(700);   
  while(Serial.available()!=0)  /* If data is available on serial port */
   {
     rx_buff1[i] = Serial.read(); /* Copy data to a buffer for later use */
     i++;
   } 
  if(strstr(rx_buff1,"+CMGL:")!=0)  /* If buffer contains +CMGL: */
  {
    s = strstr(rx_buff1,":"); /* Pointer to : */
    return atoi(s+1); /* Return message index extracted, in int form */
  }
  return 0;
}

void readSMS(int messageIndex, char *data)
{
  int8_t k = 0;
  char *st;
  char index[4];
  char rx_buff2[100];  
  memset(rx_buff2,0,100);
  flush_rx_buffer();
  Serial.println("AT+CMGF=1");  /* Select message format as text */
  delay(700);
  flush_rx_buffer();
  Serial.print("AT+CMGR="); /* Read message at specified index */
  itoa(messageIndex, index, 10);
  Serial.println(index);
  delay(700);
  while(Serial.available()!=0)  /* If data is available on serial port */
   {
     rx_buff2[k] = Serial.read(); /* Copy data to a buffer for later use */
     k++;
   } 
  if( (st = strstr(rx_buff2,"+CMGR:")) !=0 )  /* If buffer contains +CMGR: */
  {
    if( (st = strstr(st,"\r\n"))!=0 ) /* If buffer contains \r\n */
    {
      Serial.println(st); /* Print message received */
      for(int8_t j = 0; j<40 ; j++)
      {
       *(data + j)  = *(st++);  /* Store message received in data buffer */
      }
    }
  }
}

void setup() {
  Serial.begin(9600);  /* Define baud rate for serial communication */
  //pinMode(2, OUTPUT); /* LED to show whether fan is on or off */      
  //digitalWrite(2, LOW);
}

void loop() {  
  //int16_t temp_adc_val;
  //float temp_val;
  //temp_adc_val = analogRead(A6);  /* Read temperature from LM35 */
  //temp_val = (temp_adc_val * 3.22);
  //temp_val = (temp_val/10);
  //Serial.print("Temperature = ");
  //Serial.print(temp_val);
  //Serial.print(" Degree Celsius\n");  
    //if(temp_val>35)
    //{                     
        char number[11] = "9967895610"; /* Mobile number to be called */
        memset(data, 0, 40);   
        Serial.println("Accident has occurred");
        Serial.println("Calling..");
        sprintf(data,"ATD%s;",number);
        Serial.println(data);
        memset(data,0,40);
        delay(25000);
        Serial.println("ATH");
        delay(700);
        int8_t messageIndex,count = 0;
        messageIndex = UnreadSMS(); /* Check if new message available */
        Serial.print(messageIndex);        
              while( (messageIndex < 1) && !strstr( data,"Cool down") ) /* No new unread message */
              {
                  if(count == 5)
                  {
                      messageIndex = UnreadSMS();
                      break;
                  }
                  count++;
                  delay(5000);
                  messageIndex = UnreadSMS();
              }
              while(messageIndex > 0 )  /* New unread message available */
              {
                  memset(data,0, 40);
                  delay(1000);  
                  flush_rx_buffer();                
                  readSMS(messageIndex, data);
                  if(strstr(data,"Cool down"))  /* If message received has Cool down */
                    {
                        //Serial.print("Fan ON");
                        digitalWrite(2, HIGH);  /* Turn on LED to show fan is on */
                        memset(data, 0, 40);
                    }
                  messageIndex = UnreadSMS();
              }         
        delay(10000);
    }
   // else
        //Serial.print("\n");
        //Serial.print("Everything is fine\n");
        //digitalWrite(2, LOW); /* Turn off LED to show fan is off */ 
    //}
  //delay(3000);

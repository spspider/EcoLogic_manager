
#ifdef use_telegram
/*
  Name:        echoBot.ino
  Created:     12/21/2017
  Author:      Stefano Ledda <shurillu@tiscalinet.it>
  Description: a simple example that check for incoming messages
              and reply the sender with the received message
*/
#include "CTBot.h"
CTBot myBot;
CTBotInlineKeyboard myKbd;

void setup_telegram() {
  // initialize the Serial


  // set the telegram bot token
  myBot.setTelegramToken(BOTtoken);

  // check if all things are ok
  if (myBot.testConnection())
    Serial.println("\ntestConnection OK");
  else
    Serial.println("\ntestConnection NOK");
  for (char i = 0; i < char(nWidgets); i++) {
    if (pinmode[i] == 2) {
      Serial.println("adding button");

      char descr_on[20];
      sprintf(descr_on, "%s :on", descr[i], 20);
      char descr_off[20];
      sprintf(descr_off, "%s :off", descr[i], 20);

      myKbd.addButton(descr_on, descr_on, CTBotKeyboardButtonQuery);
      myKbd.addButton(descr_off, descr_off, CTBotKeyboardButtonQuery);
      myKbd.addRow();
    }
  }
}

void loop_telegram_char() { //char
  // a variable to store telegram message data
  TBMessage msg;
  if (CTBotMessageText == myBot.getNewMessage(msg)) {
    // ...forward it to the sender
    char messageText[256];
    strcpy(messageText, msg.text.c_str());

    char* colonIndex = strchr(messageText, ':');

    // Extract the substrings before and after the colon
    if (colonIndex != NULL) {
      // Extract the substrings before and after the colon
      char part1[50];
      strncpy(part1, messageText, colonIndex - messageText);
      part1[colonIndex - messageText] = '\0';

      char part2[50];
      strcpy(part2, colonIndex + 1);

      char Topic = atoi(part1);
      Serial.print("Topic:");
      Serial.println(Topic);
      Serial.print("value:");
      Serial.println(part2);

      callback_scoket(Topic, atoi(part2));

      // Clear the contents of part1 and part2
      memset(part1, 0, sizeof(part1));
      memset(part2, 0, sizeof(part2));
    }

    char fullanswer[256] = "";
    for (int i = 0; i < nWidgets; i++) {
      char answer[50];
      sprintf(answer, "%s: %.2f\n", descr[i], get_new_pin_value(i));
      strcat(fullanswer, answer);
    }
    myBot.sendMessage(msg.sender.id, fullanswer);

    // Clear the contents of fullanswer
    memset(fullanswer, 0, sizeof(fullanswer));

    delay(500);
  }
}

void loop_telegram() { // String way
  // a variable to store telegram message data
  TBMessage msg;
  //    if there is an incoming message...
  if (CTBotMessageText == myBot.getNewMessage(msg)) {
    // ...forward it to the sender
    String messageText = msg.text;
    char colonIndex = messageText.indexOf(':');
    // Extract the substrings before and after the colon
    if (colonIndex != -1) {
      // Extract the substrings before and after the colon
      String part1 = messageText.substring(0, colonIndex);
      String part2 = messageText.substring(colonIndex + 1);
      char Topic = (char) part1.toInt();
//      Serial.print("Topic:");
//      Serial.println(Topic, DEC);
//      Serial.print("value:");
//      Serial.println(part2);

      callback_scoket(Topic, part2.toInt());
    }

    //////////////////STRING WORKING//////////////
    String fullanswer = "";
    // Assuming nWidgets is defined somewhere
    for (int i = 0; i < nWidgets; i++) {
      String valueStr = String(get_new_pin_value(i), 2);
      fullanswer += String(descr[i]) + ": " + valueStr + "\n";
    }
    /////////////////////////////////////////

    myBot.sendMessage(msg.sender.id, fullanswer);
    delay(500);
  }

}
#endif

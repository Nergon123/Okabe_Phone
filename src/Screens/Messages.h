#pragma once
#include "../GlobalVariables.h"
void messages();
void inbox(bool outbox);
bool messageActivity(Contact contact, String date = "", String subject = "", String content = "", int index = -1, bool outcoming = false, bool sms = true);
void messageActivityOut(Contact contact, String subject = "", String content = "", bool sms = true);
bool messageActivity(Message message);
void parseMessages(Message *&msgs, int &count);
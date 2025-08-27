#pragma once
#include "../Input/Input.h"
#include "../GlobalVariables.h"
#include "../Connectivity/SIM.h"
#include "../System/FontManagement.h"

struct Contact;
void makeCall(Contact contact);
void contactss();
void incomingCall(Contact contact);
void callActivity(Contact contact);
void editContact(Contact contact);
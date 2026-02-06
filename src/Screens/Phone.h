#pragma once
#include "Connectivity/SIM.h"
#include "GlobalVariables.h"
#include "Input/Input.h"
#include "System/FontManagement.h"

struct Contact;
void makeCall(Contact contact);
void contactss();
void incomingCall(Contact contact);
void callActivity(Contact contact);
void editContact(Contact contact);
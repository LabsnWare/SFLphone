/*
 *  Copyright (C) 2004, 2005, 2006, 2009, 2008, 2009, 2010 Savoir-Faire Linux Inc.
 *  Author: Emmanuel Milou <emmanuel.milou@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "instantmessagingtest.h"

#define MAXIMUM_SIZE	10
#define DELIMITER_CHAR	"\n\n"

using std::cout;
using std::endl;

void InstantMessagingTest::setUp()
{
    _im = new sfl::InstantMessaging ();
    _im->init ();
}

void InstantMessagingTest::testSaveSingleMessage ()
{
    _debug ("-------------------- InstantMessagingTest::testSaveSingleMessage --------------------\n");

    std::string input, tmp;
    std::string callID = "testfile1.txt";
    std::string filename = "im:";

    // Open a file stream and try to write in it
    CPPUNIT_ASSERT (_im->saveMessage ("Bonjour, c'est un test d'archivage de message", "Manu", callID, std::ios::out)  == true);

    filename.append(callID);
    // Read it to check it has been successfully written
    std::ifstream testfile (filename.c_str (), std::ios::in);
    CPPUNIT_ASSERT (testfile.is_open () == true);

    while (!testfile.eof ()) {
        std::getline (testfile, tmp);
        input.append (tmp);
    }

    testfile.close ();
    CPPUNIT_ASSERT (input == "[Manu] Bonjour, c'est un test d'archivage de message");
}

void InstantMessagingTest::testSaveMultipleMessage ()
{
    _debug ("-------------------- InstantMessagingTest::testSaveMultipleMessage --------------------\n");

    std::string input, tmp;
    std::string callID = "testfile2.txt";
    std::string filename = "im:";

    // Open a file stream and try to write in it
    CPPUNIT_ASSERT (_im->saveMessage ("Bonjour, c'est un test d'archivage de message", "Manu", callID, std::ios::out)  == true);
    CPPUNIT_ASSERT (_im->saveMessage ("Cool", "Alex", callID, std::ios::out || std::ios::app)  == true);

    filename.append(callID);
    // Read it to check it has been successfully written
    std::ifstream testfile (filename.c_str (), std::ios::in);
    CPPUNIT_ASSERT (testfile.is_open () == true);

    while (!testfile.eof ()) {
        std::getline (testfile, tmp);
        input.append (tmp);
    }

    testfile.close ();
    printf ("%s\n", input.c_str());
    CPPUNIT_ASSERT (input == "[Manu] Bonjour, c'est un test d'archivage de message[Alex] Cool");
}

void InstantMessagingTest::testSplitMessage ()
{

    _im->setMessageMaximumSize(10);
    unsigned int maxSize = _im->getMessageMaximumSize();

    /* A message that does not need to be split */
    std::string short_message = "Salut";
    std::vector<std::string> messages = _im->split_message (short_message);
    CPPUNIT_ASSERT (messages.size() == short_message.length() / maxSize + 1);
    CPPUNIT_ASSERT (messages[0] == short_message);

    /* A message that needs to be split into two messages */
    std::string long_message = "A message too long";
    messages = _im->split_message (long_message);
    int size = messages.size ();
    int i = 0;
    CPPUNIT_ASSERT (size == (int) (long_message.length() / maxSize + 1));

    /* If only one element, do not enter the loop */
    for (i = 0; i < size - 1; i++) {
        CPPUNIT_ASSERT (messages[i] == long_message.substr ( (maxSize * i), maxSize) + DELIMITER_CHAR);
    } 

    /* Works for the last element, or for the only element */
    CPPUNIT_ASSERT (messages[size- 1] == long_message.substr (maxSize * (size-1)));

    /* A message that needs to be split into four messages */
    std::string very_long_message = "A message that needs to be split into many messages";
    messages = _im->split_message (very_long_message);
    size = messages.size ();

    /* If only one element, do not enter the loop */
    for (i = 0; i < size - 1; i++) {
        CPPUNIT_ASSERT (messages[i] ==very_long_message.substr ( (maxSize * i), maxSize) + DELIMITER_CHAR);
    }

    /* Works for the last element, or for the only element */
    CPPUNIT_ASSERT (messages[size- 1] == very_long_message.substr (maxSize * (size-1)));
}

void InstantMessagingTest::tearDown()
{
    delete _im;
    _im = 0;
}
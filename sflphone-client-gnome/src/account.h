/*
 *  Copyright (C) 2004, 2005, 2006, 2009, 2008, 2009, 2010 Savoir-Faire Linux Inc.
 *  Author: Pierre-Luc Bacon <pierre-luc.bacon@savoirfairelinux.com>
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

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include "codec.h"

/** @enum account_state_t
  * This enum have all the states an account can take.
  */
typedef enum
{
  /** Invalid state */
   ACCOUNT_STATE_INVALID = 0,
   /** The account is registered  */
   ACCOUNT_STATE_REGISTERED,
   /** The account is not registered */
   ACCOUNT_STATE_UNREGISTERED,
   /** The account is trying to register */
   ACCOUNT_STATE_TRYING,
   /** Error state. The account is not registered */
   ACCOUNT_STATE_ERROR,
   /** An authentification error occured. Wrong password or wrong username. The account is not registered */
   ACCOUNT_STATE_ERROR_AUTH,
   /** The network is unreachable. The account is not registered */
   ACCOUNT_STATE_ERROR_NETWORK,
   /** Host is unreachable. The account is not registered */
   ACCOUNT_STATE_ERROR_HOST,
   /** Stun server configuration error. The account is not registered */
   ACCOUNT_STATE_ERROR_CONF_STUN,
   /** Stun server is not existing. The account is not registered */
   ACCOUNT_STATE_ERROR_EXIST_STUN,
   /** IP profile status **/
   IP2IP_PROFILE_STATUS
} account_state_t;

/** @struct account_t
  * @brief Account information.
  * This struct holds information about an account.  All values are stored in the
  * properties GHashTable except the accountID and state.  This match how the
  * server internally works and the dbus API to save and retrieve the accounts details.
  *
  * To retrieve the Alias for example, use g_hash_table_lookup(a->properties, ACCOUNT_ALIAS).
  */
typedef struct  {
        gchar * accountID;
        account_state_t state;
        gchar * protocol_state_description;
        guint protocol_state_code;
        GHashTable * properties;
        GPtrArray * credential_information;

        /* The codec list */
        codec_library_t* codecs;
        guint _messages_number;
} account_t;

/**
 * @param accountID The accountID.
 * @return a new and initialized account object.
 */
account_t* account_new(gchar* accountID);

/**
 * @param account The account to free
 * @postcondition All of the elements within the structure as well as the structure itself will be freed.
 */
void account_free(account_t* account);


#endif
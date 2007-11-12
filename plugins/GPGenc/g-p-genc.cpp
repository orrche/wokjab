/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "g-p-genc.hpp"
#define fail_if_err(err)					\
  do								\
    {								\
      if (err)							\
        {							\
          fprintf (stderr, "%s:%d: %s: %s\n",			\
                   __FILE__, __LINE__, gpgme_strsource (err),	\
		   gpgme_strerror (err));			\
          return (1);						\
        }							\
    }								\
  while (0)



gpgme_error_t
passphrase_cb (void *c, const char *uid_hint, const char *passphrase_info,
	       int last_was_bad, int fd)
{
	std::string *s;
	s = (std::string*)c;
	std::cout << "######" << s << std::endl;
  write (fd, (*s+"\n").data(), s->size()+1);
  return 0;
}

GPGenc::GPGenc(WLSignal *wls) : WoklibPlugin(wls)
{
	/* DISABLED FOR NOW */
	
#if 0
	gpgme_check_version (NULL);
	setlocale (LC_ALL, "");
	gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifndef HAVE_W32_SYSTEM
	gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif
	gpgme_error_t err;
	err = gpgme_engine_check_version (GPGME_PROTOCOL_OpenPGP);
	if ( err ) 
	{
		fprintf (stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource (err), gpgme_strerror(err));
	}
	else
	{
		WokXMLTag form("form");
		WokXMLTag &x = form.AddTag("x");
		x.AddAttr("xmlns", "jabber:x:data");
		x.AddAttr("type", "submit");
		
		WokXMLTag &ppfield = x.AddTag("field");
		ppfield.AddAttr("type", "text-private");
		ppfield.AddAttr("var", "passphrase");
		
		WokXMLTag &keyfield = x.AddTag("field");
		keyfield.AddAttr("type", "list-single");
		keyfield.AddAttr("var", "key");
		WokXMLTag &option = keyfield.AddTag("option");
		option.AddTag("value").AddText("default");
		
		wls->SendSignal("Jabber jabber:x:data Init", form);
		
		EXP_SIGHOOK(form.GetAttr("signal"), &GPGenc::Setup, 500);
	}
#endif
}

static void
check_result (gpgme_sign_result_t result, gpgme_sig_mode_t type)
{
  if (result->invalid_signers)
    {
      fprintf (stderr, "Invalid signer found: %s\n",
	       result->invalid_signers->fpr);
      exit (1);
    }
  if (!result->signatures || result->signatures->next)
    {
      fprintf (stderr, "Unexpected number of signatures created\n");
      exit (1);
    }
  if (result->signatures->type != type)
    {
      fprintf (stderr, "Wrong type of signature created\n");
      exit (1);
    }
  if (result->signatures->pubkey_algo != GPGME_PK_DSA)
    {
      fprintf (stderr, "Wrong pubkey algorithm reported: %i\n",
	       result->signatures->pubkey_algo);
      exit (1);
    }
  if (result->signatures->hash_algo != GPGME_MD_SHA1)
    {
      fprintf (stderr, "Wrong hash algorithm reported: %i\n",
	       result->signatures->hash_algo);
      exit (1);
    }
  if (result->signatures->sig_class != 1)
    {
      fprintf (stderr, "Wrong signature class reported: %u\n",
	       result->signatures->sig_class);
      exit (1);
    }
  //~ if (strcmp ("A0FF4590BB6122EDEF6E3C542D727CC768697734",
	      //~ result->signatures->fpr))
    //~ {
      //~ fprintf (stderr, "Wrong fingerprint reported: %s\n",
	       //~ result->signatures->fpr);
      //~ exit (1);
    //~ }
}

std::string
print_data (gpgme_data_t dh)
{
#define BUF_SIZE 512
	char buf[BUF_SIZE + 1];
	int ret;
	std::string to_ret;
	
	ret = gpgme_data_seek (dh, 0, SEEK_SET);
//  if (ret)
 //   fail_if_err (gpgme_err_code_from_errno (errno));
	while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
	{
		to_ret += std::string(buf, ret);
	}
	to_ret = to_ret.substr(to_ret.find("\n\n")+1);
	to_ret = to_ret.substr(0, to_ret.find("-----END PGP"));
	
	
	return to_ret;
//  if (ret < 0)
 //   fail_if_err (gpgme_err_code_from_errno (errno));
}

int
GPGenc::Setup(WokXMLTag *tag)
{	
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("x").GetTagList("field").begin() ; xiter != tag->GetFirstTag("x").GetTagList("field").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("var") == "passphrase")
		{
			passphrase = (*xiter)->GetFirstTag("value").GetBody();
		}
		if ( (*xiter)->GetAttr("var") == "key" )
		{
			
		}
	}
	
	
	gpgme_error_t err;
	err = gpgme_new (&ctx);
	fail_if_err(err);
	gpgme_set_protocol (ctx, GPGME_PROTOCOL_OpenPGP);
	gpgme_set_textmode (ctx, 1);
	gpgme_set_armor (ctx, 1);
	gpgme_set_passphrase_cb (ctx, passphrase_cb, (void*)(&passphrase));
	
	EXP_SIGHOOK("Jabber XML Presence Send", &GPGenc::Presence, 950);
	EXP_SIGHOOK("Jabber XML Message Send", &GPGenc::OutMessage, 950);
	EXP_SIGHOOK("Jabber XML Object message", &GPGenc::Message, 1);
	EXP_SIGHOOK("Jabber XML Presence", &GPGenc::InPresence, 1);
//	EXP_SIGHOOK("Jabber XML Message xmlns jabber:x:encrypted", &GPGenc::Encrypted, 1);
	
	return 1;
}

int
GPGenc::OutMessage(WokXMLTag *tag)
{
	if ( fingerprints.find(tag->GetAttr("session")) != fingerprints.end() )
	{
		if ( fingerprints[tag->GetAttr("session")].find(tag->GetFirstTag("message").GetAttr("to")) != fingerprints[tag->GetAttr("session")].end() )
		{
			gpgme_error_t err;
			gpgme_data_t in, out;
			gpgme_key_t key[2] = { NULL, NULL };
			gpgme_encrypt_result_t result;

			std::string body = tag->GetFirstTag("message").GetFirstTag("body").GetBody();
			err = gpgme_data_new_from_mem (&in, body.data(), body.size(), 0);
			fail_if_err (err);

			err = gpgme_data_new (&out);
			fail_if_err (err);

			err = gpgme_get_key (ctx, fingerprints[tag->GetAttr("session")][tag->GetFirstTag("message").GetAttr("to")].c_str(), &key[0], 0);
			fail_if_err (err);

			err = gpgme_op_encrypt_sign (ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
			fail_if_err (err);
			result = gpgme_op_encrypt_result (ctx);
			
			if (result->invalid_recipients)
			{
				fprintf (stderr, "Invalid recipient encountered: %s\n",
				result->invalid_recipients->fpr);
				exit (1);
			}
			WokXMLTag &xtag = tag->GetFirstTag("message").AddTag("x");
			xtag.AddAttr("xmlns", "jabber:x:encrypted");
			xtag.AddText(print_data(out));
			tag->GetFirstTag("message").GetFirstTag("body").RemoveBody();
			tag->GetFirstTag("message").GetFirstTag("body").AddText("This message is encrypted");
			gpgme_key_unref (key[0]);
			gpgme_key_unref (key[1]);
			gpgme_data_release (in);
			gpgme_data_release (out);

		}
	}
	
	return 1;
}

int
GPGenc::InPresence(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("presence").GetTagList("x").begin() ; xiter != tag->GetFirstTag("presence").GetTagList("x").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("xmlns") == "jabber:x:signed")
		{
			gpgme_error_t err;
			gpgme_data_t sig, text;
			gpgme_verify_result_t result;

			std::string sig_data = "-----BEGIN PGP MESSAGE-----\nVersion: GnuPG v2.0.7 (GNU/Linux)\n\n" + (*xiter)->GetBody()+ "-----END PGP MESSAGE-----\n";
			std::string sig_text = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
			
			/* Checking a valid message.  */
			err = gpgme_data_new_from_mem (&text, sig_text.data(), sig_text.size(), 0);
			fail_if_err (err);
			err = gpgme_data_new_from_mem (&sig, sig_data.data(), sig_data.size(), 0);
			fail_if_err (err);
			err = gpgme_op_verify (ctx, sig, text, NULL);
			fail_if_err (err);
			result = gpgme_op_verify_result (ctx);
			
			fingerprints[tag->GetAttr("session")][tag->GetFirstTag("presence").GetAttr("from")] = result->signatures->fpr;
//			check_result (result, 0, "A0FF4590BB6122EDEF6E3C542D727CC768697734", GPG_ERR_NO_ERROR, 1);

		}
	}
	
	return 1;	
}


int
GPGenc::Presence(WokXMLTag *tag)
{
#warning need to read up on how to check for errors here ...
	gpgme_error_t err;
	gpgme_data_t in, out;
	gpgme_sign_result_t result;
	
	
	std::string childstr = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
	
	err = gpgme_data_new_from_mem (&in, childstr.data(), childstr.size(), 0);
	fail_if_err(err);
	
	err = gpgme_data_new (&out);
	fail_if_err (err);
	err = gpgme_op_sign (ctx, in, out, GPGME_SIG_MODE_DETACH);
	fail_if_err (err);
	result = gpgme_op_sign_result (ctx);
	check_result (result, GPGME_SIG_MODE_DETACH);
	WokXMLTag &xtag = tag->GetFirstTag("presence").AddTag("x");
	xtag.AddAttr("xmlns", "jabber:x:signed");
	xtag.AddText(print_data(out));
	
	gpgme_data_release (out);
	
	return 1;
}


int
GPGenc::Message(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("message").GetTagList("x").begin() ; xiter != tag->GetFirstTag("message").GetTagList("x").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("xmlns") == "jabber:x:encrypted")
		{
			gpgme_error_t err;
			gpgme_data_t in, out;
			gpgme_decrypt_result_t result;

			std::string body = "-----BEGIN PGP MESSAGE-----\nVersion: GnuPG v2.0.7 (GNU/Linux)\n\n" + (*xiter)->GetBody()+ "\n-----END PGP MESSAGE-----\n";
			err = gpgme_data_new_from_mem (&in, body.data(), body.size(), 0);
			fail_if_err (err);

			err = gpgme_data_new (&out);
			fail_if_err (err);

			err = gpgme_op_decrypt (ctx, in, out);
			fail_if_err (err);
			result = gpgme_op_decrypt_result (ctx);
			if (result->unsupported_algorithm)
			{
				fprintf (stderr, "%s:%i: unsupported algorithm: %s\n",
					__FILE__, __LINE__, result->unsupported_algorithm);
				return 1;
			}
			
			tag->GetFirstTag("message").GetFirstTag("body").RemoveBody();
			tag->GetFirstTag("message").GetFirstTag("body").AddText(print_data (out));
			
			gpgme_data_release (in);
			gpgme_data_release (out);
			
			
		}
	}
	return 1;
}


int
GPGenc::Encrypted(WokXMLTag *tag)
{
	tag->GetFirstTag("message").GetFirstTag("body").AddText("... wokjab not yet able to decrypt");
	
	return 1;
}

//***************************************************************************************************
//   Name: types.h
//   About: defining some stuff here ..
//   Author: XerXes911. <darklinkcompany@gmail.com>
//   License: please view COPYRIGHT file in the previous folder.
//   Notes: This product uses parts of the iMatix SFL,
//   Copyright © 1991-2000 iMatix Corporation <http://www.imatix.com>
//***************************************************************************************************

typedef unsigned char   byte;           /*  Single unsigned byte = 8 bits    */

typedef struct ZENTP {
   char *strSmtpServer;
   char *strMessageBody;
   char *strHtmlMessageBody;
   char *strSubject;
   char *strSenderUserId;
   char *strFullSenderUserId;          /* to be filled with: "realname" <e-mail> */
   char *strDestUserIds;
   char *strFullDestUserIds;           /* to be filled with: "realname" <e-mail> */
   char *strCcUserIds;
   char *strFullCcUserIds;             /* to be filled with: "realname" <e-mail> */
   char *strBccUserIds;
   char *strFullBccUserIds;
   char *strRetPathUserId;
   char *strRrcpUserId;
   char *strMsgComment;
   char *strMailerName;
   char *strBinFiles;
   char *strTxtFiles;
   char strlast_smtp_message[513];
   int  debug;
   char *strDebugFile;
   int  mime;
   int  encode_type;
   int  connect_retry_cnt;
   int  retry_wait_time;
   char *strCharSet;                    /* Character Set (default is US-ASCII)*/
} ZENTP;

typedef struct DESCR{                        /*  Memory descriptor                */
    size_t size;                        /*    Size of data part              */
    byte  *data;                        /*    Data part follows here         */
} DESCR;

# Very-Simple-FTP
  
  A very simple protocol based on SOCKET API

## INTRODUCTION

   SFTP(RFC 913) is a simple file transfer protocol.  It fills the need of people wanting a protocol that allows them to transfer and manage files over networks. SFTP supports user access control, file transfers, directory listing, directory changing, file renaming and deleting.

## THE PROTOCOL

   VSFTP is used by opening a TCP connection to the remote hosts'
   VSFTP port (50001 decimal).  You then send VSFTP commands and
   wait for replies.  VSFTP commands sent to the remote server are
   always 4 ASCII letters (of any case) followed by a space, the
   argument(s), and a NULL.  The argument can sometimes be null
   in which case the command is just 4 characters followed by
   NULL.  Replies from the server are always a response character
   followed immediately by an ASCII message string terminated by a
   NULL.  A reply can also be just a response character and a
   NULL.
   
   command : cmd [SPACE args] NULL
   
   cmd : USER!PASS!LIST!RETR!KILL!DONE 
   
   response : response-code [message] NULL 
   
   response-code: +|-|!|#
   
   message can contain CRLF

   Commands that can be sent to the server are listed below.  The
   server replies to each command with one of the possible response
   codes listed under each message.  Along with the response, the
   server should optionally return a message explaining the error
   in more detail.  Example message texts are listed but do not
   have to be followed.  All characters used in messages are ASCII.

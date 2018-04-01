// =---------------------------------------------------------------------------
// p a g e _ w e b _ s t a t s . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Jan 23   nic   Created.
//


#include "cgi_id_subsystems.h"
#include "pages.h"

// =---------------------------------------------------------------------------
// P a g e _ W e b _ S t a t s
//
// =---------------------------------------------------------------------------
int Page_Web_Stats ( )
{
   SS_HTML_Start_Page ( "Web Server Statistics" );

   SS_HTML_Heading_Tag ( "Web Server Statistics (Form Stats)", 1 );

   SS_HTML_Table_Tag ( );

   SS_HTML_Write ( "<TR><TD>cgiServerSoftware   </TD><TD> %s </TD></TR>", Web_Vars.cgiServerSoftware  );
   SS_HTML_Write ( "<TR><TD>cgiServerName       </TD><TD> %s </TD></TR>", Web_Vars.cgiServerName      );
   SS_HTML_Write ( "<TR><TD>cgiGatewayInterface </TD><TD> %s </TD></TR>", Web_Vars.cgiGatewayInterface);
   SS_HTML_Write ( "<TR><TD>cgiServerProtocol   </TD><TD> %s </TD></TR>", Web_Vars.cgiServerProtocol  );
   SS_HTML_Write ( "<TR><TD>cgiServerPort       </TD><TD> %s </TD></TR>", Web_Vars.cgiServerPort      );
   SS_HTML_Write ( "<TR><TD>cgiRequestMethod    </TD><TD> %s </TD></TR>", Web_Vars.cgiRequestMethod   );
   SS_HTML_Write ( "<TR><TD>cgiPathInfo         </TD><TD> %s </TD></TR>", Web_Vars.cgiPathInfo        );
   SS_HTML_Write ( "<TR><TD>cgiPathTranslated   </TD><TD> %s </TD></TR>", Web_Vars.cgiPathTranslated  );
   SS_HTML_Write ( "<TR><TD>cgiScriptName       </TD><TD> %s </TD></TR>", Web_Vars.cgiScriptName      );
   SS_HTML_Write ( "<TR><TD>cgiQueryString      </TD><TD> %s </TD></TR>", Web_Vars.cgiQueryString     );
   SS_HTML_Write ( "<TR><TD>cgiRemoteHost       </TD><TD> %s </TD></TR>", Web_Vars.cgiRemoteHost      );
   SS_HTML_Write ( "<TR><TD>cgiRemoteAddr       </TD><TD> %s </TD></TR>", Web_Vars.cgiRemoteAddr      );
   SS_HTML_Write ( "<TR><TD>cgiAuthType         </TD><TD> %s </TD></TR>", Web_Vars.cgiAuthType        );
   SS_HTML_Write ( "<TR><TD>cgiRemoteUser       </TD><TD> %s </TD></TR>", Web_Vars.cgiRemoteUser      );
   SS_HTML_Write ( "<TR><TD>cgiRemoteIdent      </TD><TD> %s </TD></TR>", Web_Vars.cgiRemoteIdent     );
   SS_HTML_Write ( "<TR><TD>cgiContentType      </TD><TD> %s </TD></TR>", Web_Vars.cgiContentType     );
   SS_HTML_Write ( "<TR><TD>cgiAccept           </TD><TD> %s </TD></TR>", Web_Vars.cgiAccept          );
   SS_HTML_Write ( "<TR><TD>cgiUserAgent        </TD><TD> %s </TD></TR>", Web_Vars.cgiUserAgent       );

   SS_HTML_End_Table_Tag ();

   SS_HTML_End_Page  ( );

   return 0;
}

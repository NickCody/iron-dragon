// =---------------------------------------------------------------------------
// p a g e _ n e w s _ a d m i n . c
// 
//   (C) 1999 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//    
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   00 Mar 24   nic   Created.
//

#include <stdlib.h>
#include "cgi_id_subsystems.h"
#include "pages.h"
#include "databases.h"
#include "cgic.h"

// =---------------------------------------------------------------------------
// Page_News_Admin
// =---------------------------------------------------------------------------
int Page_News_Admin ( )
{
   char              Direct [VARIABLE_BYTES];
   char              Record_Number [VARIABLE_BYTES];
   char              Link   [256];
   db_news_articles  Record;
   db_metastructure  ms;
   int               ret = 0;
   uint_32           recnum;
   int               Write_Articles = 1;

   SS_HTML_Start_Page ( "RE News Administration" );
   SS_HTML_Heading_Tag ( "RE News Administration", 1 );

   ret = SS_DB_Initialize_MetaStructure ( &ms, sizeof(db_news_articles), 0, 0 );
   ret = ret == 0 ? SS_DB_Create_Open ( &ms, "", DB_Name_News_Articles ) : -2;

   if ( ret == 0 )
   {
      if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Direct", Direct, VARIABLE_BYTES) )
      {
         switch ( atoi(Direct) )
         {
            // FORM: New Article
            //
            case 1:
            {
               Write_Script_Form ( "NewsAdmin" );
               SS_HTML_Input_Hidden ( "Direct", "3" );

               SS_Port_ZeroMemory ( &Record, sizeof(db_news_articles) );

               SS_Port_Get_DateTime_String ( Record.Publish_Date );
               strcpy ( Record.Author, Page_Info.Username );
               Write_News_Article_Inputs ( &Record );
               SS_HTML_End_Form();
               break;
            }
         
            // FORM: Edit Article (Requires Recnum)
            //
            case 2:
               if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Recnum", Record_Number, VARIABLE_BYTES) )
               {
                  recnum = atoi(Record_Number);
                  ret = SS_DB_Get_Record ( &ms, recnum, &Record );

                  if ( ret == 0 )
                  {
                     Write_Script_Form ( "NewsAdmin" );
                     SS_HTML_Input_Hidden ( "Direct", "4" );
                     SS_HTML_Input_Hidden ( "Recnum", Record_Number );
                     Write_News_Article_Inputs ( &Record );
                     SS_HTML_End_Form();
                  }
               }
               else
               {
                  SS_HTML_WriteP ( "Record number was not specified for Article Edit operation." );
               }

               break;

            // SUBMIT: Add New Article
            case 3:
               Action_Gather_News_Article_Fields ( &Record );
               ret = SS_DB_Add_Record ( &ms, &Record, &recnum );
               break;

            // SUBMIT: Edit Article
            // 
            case 4:
               Action_Gather_News_Article_Fields ( &Record );

               if ( cgiFormNotFound != cgiFormStringNoNewlines ( "Recnum", Record_Number, VARIABLE_BYTES) )
               {
                  recnum = atoi(Record_Number);
                  ret = SS_DB_Set_Record ( &ms, recnum, &Record );
               }
               else
               {
                  SS_HTML_WriteP ( "Record number was not specified for Article Submit operation." );
               }

               break;

            default:
               SS_HTML_WriteP ( "Invalid Direct page argument (%s).", Direct );
               break;
         }

         if ( atoi(Direct) == 3 || atoi(Direct) == 4 )
         {
            // Report error or success
            //
            if ( ret == 0 )
               SS_HTML_WriteP ( "The operation was completed successfully." );
            else
               SS_HTML_WriteP ( "The operation was <i>not</i> completed successfully." );

         }
         else
         {
            Write_Articles = 0;
         }

      }

      if ( Write_Articles )
      {
         Format_Script_Link ( Link, "NewsAdmin;Direct=1" );
   
         SS_HTML_WriteP ( "<a href=\"%s\">Enter a New Article</a>", Link );

         // This is our default case
         ret = Write_Article_Summaries ( NA_CLASS_ALL, &ms );

         if ( ret != 0 )
            SS_HTML_WriteP ( "We could not properly retrieve the article list." );
      }
   }
   else
   {
      SS_HTML_WriteP ( "A database error occurred while accessing the news database." );
   }

   SS_DB_Close ( &ms );

   Write_Admin_Footer ( );
   SS_HTML_End_Page ( );

   return ret;
}



/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Zepp 1998    *
 *    _/_/_/_/      _/          _/  _/             Version #: 4.3          *
 *   _/      _/      _/_/_/     _/    _/     _/                            *
 *                                                                         *
 *                        http://ackmud.nuc.net/                           *
 *                        zenithar@ackmud.nuc.net                          *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
/***************************************************************************
 * _/_/_/_/  _/    _/  _/_/_/_/ _/_/_/_/ AckFUSS is modified ACK!MUD 4.3.1 *
 * _/        _/    _/  _/       _/       copyright Matt Goff (Kline) 2008  *
 * _/_/      _/    _/  _/_/_/_/ _/_/_/_/                                   *
 * _/        _/    _/        _/       _/ Support for this code is provided *
 * _/        _/_/_/_/  _/_/_/_/ _/_/_/_/ at www.ackmud.net -- check it out!*
 ***************************************************************************/

/* Online Social Editting Module,
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 * See the file "License" for important licensing information
 */

#include "h/globals.h"

#ifndef DEC_ACT_INFO_H
#include "h/act_info.h"
#endif

#ifndef DEC_COMM_H
#include "h/comm.h"
#endif

#ifndef DEC_DB_H
#include "h/db.h"
#endif

#ifndef DEC_SSM_H
#include "h/ssm.h"
#endif

int maxSocial; /* max number of socials */

#ifndef CONST_SOCIAL
SOCIAL_TYPE *social_table;   /* and social table */

void load_social( FILE * fp, SOCIAL_TYPE *social )
{
   social->name = fread_string( fp );
   social->char_no_arg = fread_string( fp );
   social->others_no_arg = fread_string( fp );
   social->char_found = fread_string( fp );
   social->others_found = fread_string( fp );
   social->vict_found = fread_string( fp );
   social->char_auto = fread_string( fp );
   social->others_auto = fread_string( fp );
}

void load_social_table(  )
{
   FILE *fp;
   int i;

   snprintf(log_buf,(2 * MIL),"Loading %s",SOCIAL_FILE);
   log_f("%s",log_buf);

   fp = file_open( SOCIAL_FILE, "r" );

   if( !fp )
   {
      bug( "Could not open " SOCIAL_FILE " for reading.", 0 );
      exit( 1 );
   }

   if( fscanf( fp, "%d\n", &maxSocial ) < 1 )
    bugf("Didn't get a count for MAX_SOCAIALS, this is bad!");

   /*
    * IMPORTANT to use malloc so we can realloc later on
    */

   social_table = (SOCIAL_TYPE *)malloc( sizeof( SOCIAL_TYPE ) * ( maxSocial + 1 ) );

   for( i = 0; i < maxSocial; i++ )
      load_social( fp, &social_table[i] );

   /*
    * For backwards compatibility
    */

   social_table[maxSocial].name = str_dup( "" );   /* empty! */

   log_f("Done.");
   file_close( fp );

}

#endif /* CONST_SOCIAL */

void save_social( const SOCIAL_TYPE *s, FILE * fp )
{
   /*
    * get rid of (null) 
    */
   fprintf( fp, "%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n\n",
            s->name ? s->name : "",
            s->char_no_arg ? s->char_no_arg : "",
            s->others_no_arg ? s->others_no_arg : "",
            s->char_found ? s->char_found : "",
            s->others_found ? s->others_found : "",
            s->vict_found ? s->vict_found : "", s->char_auto ? s->char_auto : "", s->others_auto ? s->others_auto : "" );
}


void save_social_table(  )
{
   FILE *fp;
   int i;

   fp = file_open( SOCIAL_FILE, "w" );

   if( !fp )
   {
      bug( "Could not open " SOCIAL_FILE " for writing.", 0 );
      return;
   }

#ifdef CONST_SOCIAL  /* If old table still in use, count socials first */

   for( maxSocial = 0; social_table[maxSocial].name[0]; maxSocial++ )
      ;  /* empty */
#endif


   fprintf( fp, "%d\n", maxSocial );

   for( i = 0; i < maxSocial; i++ )
      save_social( &social_table[i], fp );

   file_close( fp );
}


/* Find a social based on name */
int social_lookup( const char *name )
{
   int i;

   for( i = 0; i < maxSocial; i++ )
      if( !str_cmp( name, social_table[i].name ) )
         return i;

   return -1;
}

/*
 * Social editting command
 */

#ifndef CONST_SOCIAL
DO_FUN(do_sedit)
{
   char cmd[MAX_INPUT_LENGTH], social[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int iSocial;
   smash_tilde( argument );
   argument = one_argument( argument, cmd );
   argument = one_argument( argument, social );

   if( !cmd[0] )
   {
      do_help(ch,"sedit");
      return;
   }

   if( !social[0] )
   {
      send_to_char( "What social do you want to operate on?\r\n", ch );
      return;
   }

   iSocial = social_lookup( social );

   if( str_cmp( cmd, "new" ) && ( iSocial == -1 ) )
   {
      send_to_char( "No such social exists.\r\n", ch );
      return;
   }

   if( !str_cmp( cmd, "delete" ) )  /* Remove a social */
   {
      int i, j;
      SOCIAL_TYPE *new_table = (SOCIAL_TYPE *)malloc( sizeof( SOCIAL_TYPE ) * maxSocial );

      if( !new_table )
      {
         send_to_char( "Memory allocation failed. Brace for impact...\r\n", ch );
         return;
      }

      /*
       * Copy all elements of old table into new table, except the deleted social
       */
      for( i = 0, j = 0; i < maxSocial + 1; i++ )
         if( i != iSocial )   /* copy, increase only if copied */
         {
            new_table[j] = social_table[i];
            j++;
         }

      free( social_table );
      social_table = new_table;

      maxSocial--;   /* Important :() */

      send_to_char( "That social is history now.\r\n", ch );

   }

   else if( !str_cmp( cmd, "new" ) )   /* Create a new social */
   {
      SOCIAL_TYPE *new_table;

      if( iSocial != -1 )
      {
         send_to_char( "A social with that name already exists\r\n", ch );
         return;
      }

      /*
       * reallocate the table
       */
      /*
       * Note that the table contains maxSocial socials PLUS one empty spot!
       */

      maxSocial++;
      new_table = (SOCIAL_TYPE *)realloc( social_table, sizeof( SOCIAL_TYPE ) * maxSocial + 1 );

      if( !new_table )  /* realloc failed */
      {
         send_to_char( "Memory allocation failed. Brace for impact.\r\n", ch );
         return;
      }

      social_table = new_table;

      social_table[maxSocial - 1].name = str_dup( social );
      social_table[maxSocial - 1].char_no_arg = str_dup( "" );
      social_table[maxSocial - 1].others_no_arg = str_dup( "" );
      social_table[maxSocial - 1].char_found = str_dup( "" );
      social_table[maxSocial - 1].others_found = str_dup( "" );
      social_table[maxSocial - 1].vict_found = str_dup( "" );
      social_table[maxSocial - 1].char_auto = str_dup( "" );
      social_table[maxSocial - 1].others_auto = str_dup( "" );

      social_table[maxSocial].name = str_dup( "" );   /* 'terminating' empty string */

      send_to_char( "New social added.\r\n", ch );

   }

   else if( !str_cmp( cmd, "show" ) )  /* Show a certain social */
   {
      snprintf( buf, MSL, "Social: %s\r\n"
               "(cnoarg) No argument given, character sees:\r\n"
               "%s\r\n\r\n"
               "(onoarg) No argument given, others see:\r\n"
               "%s\r\n\r\n"
               "(cfound) Target found, character sees:\r\n"
               "%s\r\n\r\n"
               "(ofound) Target found, others see:\r\n"
               "%s\r\n\r\n"
               "(vfound) Target found, victim sees:\r\n"
               "%s\r\n\r\n"
               "(cself) Target is character himself:\r\n"
               "%s\r\n\r\n"
               "(oself) Target is character himself, others see:\r\n"
               "%s\r\n",
               social_table[iSocial].name,
               social_table[iSocial].char_no_arg,
               social_table[iSocial].others_no_arg,
               social_table[iSocial].char_found,
               social_table[iSocial].others_found,
               social_table[iSocial].vict_found, social_table[iSocial].char_auto, social_table[iSocial].others_auto );

      send_to_char( buf, ch );
      return;  /* return right away, do not save the table */
   }

   else if( !str_cmp( cmd, "cnoarg" ) )   /* Set that argument */
   {
      free_string( social_table[iSocial].char_no_arg );
      social_table[iSocial].char_no_arg = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Character will now see nothing when this social is used without arguments.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );
   }

   else if( !str_cmp( cmd, "onoarg" ) )
   {
      free_string( social_table[iSocial].others_no_arg );
      social_table[iSocial].others_no_arg = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Others will now see nothing when this social is used without arguments.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );

   }

   else if( !str_cmp( cmd, "cfound" ) )
   {
      free_string( social_table[iSocial].char_found );
      social_table[iSocial].char_found = str_dup( argument );

      if( !argument[0] )
         send_to_char( "The character will now see nothing when a target is found.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );

   }

   else if( !str_cmp( cmd, "ofound" ) )
   {
      free_string( social_table[iSocial].others_found );
      social_table[iSocial].others_found = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Others will now see nothing when a target is found.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );

   }

   else if( !str_cmp( cmd, "vfound" ) )
   {
      free_string( social_table[iSocial].vict_found );
      social_table[iSocial].vict_found = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Victim will now see nothing when a target is found.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );
   }

   else if( !str_cmp( cmd, "cself" ) )
   {
      free_string( social_table[iSocial].char_auto );
      social_table[iSocial].char_auto = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Character will now see nothing when targetting self.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );

   }

   else if( !str_cmp( cmd, "oself" ) )
   {
      free_string( social_table[iSocial].others_auto );
      social_table[iSocial].others_auto = str_dup( argument );

      if( !argument[0] )
         send_to_char( "Others will now see nothing when character targets self.\r\n", ch );
      else
         act( "New message is now:\r\n$T\r\n", ch, NULL, argument, TO_CHAR );
   }

   else
   {
      send_to_char( "Huh? Try HELP SEDIT.\r\n", ch );
      return;
   }

   /*
    * We have done something. update social table
    */

   save_social_table(  );
}
#endif /* CONST_SOCIAL */

/*
   Copyright (c) 2015, 2016, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client_priv.h"
#include "my_default.h"
#include "mysqld_error.h"
#include <welcome_copyright_notice.h> // ORACLE_WELCOME_COPYRIGHT_NOTICE

using namespace std;

static char **defaults_argv= 0;
static char *opt_host= 0;
static char *opt_user= 0;
static uint opt_port= 0;
static uint opt_protocol= 0;
static char *opt_socket= 0;
static MYSQL mysql;
static char *password= 0;
static my_bool password_provided= FALSE;
static my_bool g_expire_password_on_exit= FALSE;
static my_bool opt_use_default= FALSE;

#if defined (_WIN32) && !defined (EMBEDDED_LIBRARY)
static char *shared_memory_base_name= default_shared_memory_base_name;
#endif

#include "sslopt-vars.h"

static const char *load_default_groups[]= { "mysql_secure_installation", "mysql", "client", 0 };

static struct my_option my_connection_options[]=
{
  {"help", '?', "Display this help and exit.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
  {"host", 'h', "Connect to host.", &opt_host,
   &opt_host, 0, GET_STR_ALLOC, REQUIRED_ARG,
   (longlong) "localhost", 0, 0, 0, 0, 0},
  {"password", 'p', "Password to connect to the server. If password is not "
   "given it's asked from the tty.", 0, 0, 0, GET_PASSWORD, OPT_ARG , 0, 0, 0,
   0, 0, 0},
#ifdef __WIN__
  {"pipe", 'W', "Use named pipes to connect to server.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
#endif
  {"port", 'P', "Port number to use for connection or 0 for default to, in "
   "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
   "/etc/services, "
#endif
   "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").", &opt_port,
   &opt_port, 0, GET_UINT, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"protocol", OPT_MYSQL_PROTOCOL,
   "The protocol to use for connection (tcp, socket, pipe, memory).",
   0, 0, 0, GET_STR,  REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#if defined (_WIN32) && !defined (EMBEDDED_LIBRARY)
  {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
   "Base name of shared memory.", &shared_memory_base_name,
   &shared_memory_base_name, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#endif
  {"socket", 'S', "Socket file to be used for connection.",
   &opt_socket, &opt_socket, 0, GET_STR_ALLOC, REQUIRED_ARG,
   0, 0, 0, 0, 0, 0},
#include "sslopt-longopts.h"
  {"user", 'u', "User for login if not root.", &opt_user,
   &opt_user, 0, GET_STR_ALLOC, REQUIRED_ARG, (longlong) "root", 0, 0, 0, 0, 0},
  {"use-default", 'D', "Execute with no user interactivity",
   &opt_use_default, &opt_use_default, 0, GET_BOOL, NO_ARG, 0, 0, 0,
   0, 0, 0},
  /* End token */
  {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};

my_bool find_temporary_password(char **p);

static void print_version(void)
{
  fprintf(stdout, "%s Ver %s, for %s on %s\n", my_progname,
	  MYSQL_SERVER_VERSION, SYSTEM_TYPE, MACHINE_TYPE);
}

static void usage()
{
  print_version();
  fprintf(stdout, ORACLE_WELCOME_COPYRIGHT_NOTICE("2013"));
  fprintf(stdout, "\nMySQL Configuration Utility.");
  fprintf(stdout, "Usage: %s [OPTIONS]\n", my_progname);
  my_print_help(my_connection_options);
  print_defaults("my", load_default_groups);
  my_print_variables(my_connection_options);
}

static void free_resources()
{
  if (opt_host)
    my_free(opt_host);
  if (opt_socket)
    my_free(opt_socket);
  if (opt_user)
    my_free(opt_user);
  if (password)
    my_free(password);
  mysql_close(&mysql);
  if (defaults_argv && *defaults_argv)
    free_defaults(defaults_argv);
}
my_bool
my_arguments_get_one_option(int optid,
                            const struct my_option *opt MY_ATTRIBUTE((unused)),
                            char *argument)
{
  switch(optid){
  case '?':
    usage();
    free_resources();
    exit(0);
  case 'p':
    if (argument)
    {
      char *start= argument;
      my_free(password);
      password= my_strdup(PSI_NOT_INSTRUMENTED,
			  argument, MYF(MY_FAE));
      while (*argument)
      {
	*argument++= 'x';               // Destroy argument
      }
      if (*start)
	start[1]= 0 ;
    }
    else
      password= get_tty_password(NullS);
    password_provided= TRUE;
    break;

#include <sslopt-case.h>
  case OPT_MYSQL_PROTOCOL:
#ifndef EMBEDDED_LIBRARY
    opt_protocol= find_type_or_exit(argument, &sql_protocol_typelib,
				    opt->name);
#endif
    break;
  case 'W':
#ifdef __WIN__
    opt_protocol = MYSQL_PROTOCOL_PIPE;
#endif
    break;
  }
  return 0;
}


/* Initialize options for the given connection handle. */
static void
init_connection_options(MYSQL *mysql)
{
  SSL_SET_OPTIONS(mysql);

  if (opt_protocol)
    mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char*) &opt_protocol);

#if defined (_WIN32) && !defined (EMBEDDED_LIBRARY)
  if (shared_memory_base_name)
    mysql_options(mysql, MYSQL_SHARED_MEMORY_BASE_NAME, shared_memory_base_name);
#endif
}


/**
  Reads the response from stdin and returns the first character.
  If global variable opt_use_default is TRUE then the default_answer is
  returned instead.
  @param    Optional message do be displayed.
  @param    default_answer Answer to be given if no interactivity is allowed.
  @return   First character of input string
*/

int get_response(const char *opt_message, int default_answer= -1)
{
  int a= 0;
  int b= 0;
  int i= 0;
  if (opt_message)
  {
    fprintf(stdout, "%s", opt_message);
    if (opt_use_default == TRUE && default_answer != -1)
    {
      fprintf(stdout, " %c \n", (char)default_answer);
      return default_answer;
    }
  }
  do
  {
    if (i == 1)
      b= a;
    a= getchar();
    i++;
  } while(a != '\n');
  return b;
}

/**
  Takes a mysql query and an optional message as arguments.
  It displays the message if provided one and then runs the query.
  If the query is run successfully, the success message is displayed.
  Else, the failure message along with the actual failure is displayed.
  If the server is not found running, the program is exited.

  @param1  query        The mysql query which is to be executed.
  @param2  opt_message  The optional message to be displayed.
*/
void execute_query_with_message(const char *query, const char *opt_message)
{
  if (opt_message)
    fprintf(stdout, "%s", opt_message);

  if (!mysql_query(&mysql, query))
    fprintf(stdout, "Success.\n\n");
  else if ((mysql_errno(&mysql) == ER_PROCACCESS_DENIED_ERROR) ||
           (mysql_errno(&mysql) == ER_TABLEACCESS_DENIED_ERROR) ||
           (mysql_errno(&mysql) == ER_COLUMNACCESS_DENIED_ERROR))
  {
    fprintf(stdout, "The user provided does not have enough permissions "
	            "to continue.\nmysql_secure_installation is exiting.\n");
    free_resources();
    exit(1);
  }
  else
    fprintf(stdout, " ... Failed! Error: %s\n", mysql_error(&mysql));

  if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR)
  {
    free_resources();
    exit(1);
  }
}

/**
  Takes a mysql query and the length of the query in bytes
  as the input. If the query fails on running, a message
  along with the failure details is displayed.

  @param1   query        The mysql query which is to be executed.
  @param2   length       Length of the query in bytes.

  return    FALSE in case of success
            TRUE  in case of failure
*/
bool execute_query(const char **query, size_t length)
{
  if (!mysql_real_query(&mysql, (const char *) *query, (ulong)length))
    return FALSE;
  else if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR)
  {
    fprintf(stdout, " ... Failed! Error: %s\n", mysql_error(&mysql));
    free_resources();
    exit(1);
  }
  if ((mysql_errno(&mysql) == ER_PROCACCESS_DENIED_ERROR) ||
      (mysql_errno(&mysql) == ER_TABLEACCESS_DENIED_ERROR) ||
      (mysql_errno(&mysql) == ER_COLUMNACCESS_DENIED_ERROR))
  {
    fprintf(stdout, "The user provided does not have enough permissions "
	            "to continue.\nmysql_secure_installation is exiting.\n");
    free_resources();
    exit(1);
  }
  return TRUE;
}

/**
  Checks if the validate_password plugin is installed and returns TRUE if it is.
*/
bool validate_password_exists()
{
  MYSQL_ROW row;
  bool res= TRUE;
  const char *query= "SELECT NAME FROM mysql.plugin WHERE NAME "
                     "= \'validate_password\'";
  if (!execute_query(&query, strlen(query)))
    DBUG_PRINT("info", ("query success!"));
  MYSQL_RES *result= mysql_store_result(&mysql);
  row= mysql_fetch_row(result);
  if (!row)
    res= FALSE;

  mysql_free_result(result);
  return res;
}

/**
  Installs validate_password plugin and sets the password validation policy.

  @return   Returns 1 on successfully setting the plugin and 0 in case of
            of any error.
*/
int install_password_validation_plugin()
{
  int reply;
  int plugin_set= 0;
  char *strength;
  bool option_read= FALSE;
  reply= get_response((const char *) "\nVALIDATE PASSWORD PLUGIN can be used "
                                     "to test passwords\nand improve security. "
				     "It checks the strength of password\nand "
				     "allows the users to set only those "
				     "passwords which are\nsecure enough. "
				     "Would you like to setup VALIDATE "
				     "PASSWORD plugin?\n\nPress y|Y for Yes, "
				     "any other key for No: ", 'y');
  if (reply == (int) 'y' || reply == (int) 'Y')
  {
#ifdef _WIN32
    const char *query_tmp;
    query_tmp= "INSTALL PLUGIN validate_password SONAME "
	       "'validate_password.dll'";
    if (!execute_query(&query_tmp, strlen(query_tmp)))
#else
    const char *query_tmp;
    query_tmp= "INSTALL PLUGIN validate_password SONAME "
	       "'validate_password.so'";
    if (!execute_query(&query_tmp, strlen(query_tmp)))
#endif
    {
      plugin_set= 1;
      while(!option_read)
      {
	reply= get_response((const char *) "\nThere are three levels of "
   "password validation policy:\n\n"
   "LOW    Length >= 8\n"
   "MEDIUM Length >= 8, numeric, mixed case, and special characters\n"
   "STRONG Length >= 8, numeric, mixed case, special characters and dictionary"
   "                  file\n\n"
   "Please enter 0 = LOW, 1 = MEDIUM and 2 = STRONG: ",'2');
	switch (reply){
	case (int ) '0':
	  strength= (char *) "LOW";
	  option_read= TRUE;
	  break;
	case (int) '1':
	  strength= (char *) "MEDIUM";
	  option_read= TRUE;
	  break;
	case (int) '2':
	  strength= (char *) "STRONG";
	  option_read= TRUE;
	  break;
	default:
	  fprintf(stdout, "\nInvalid option provided.\n");
	}
      }
      char *query, *end;
      int tmp= sizeof("SET GLOBAL validate_password_policy = ") + 3;
      size_t strength_length= strlen(strength);
      /*
	query string needs memory which is atleast the length of initial part
	of query plus twice the size of variable being appended.
      */
      query= (char *)my_malloc(PSI_NOT_INSTRUMENTED,
	                       (strength_length * 2 + tmp) * sizeof(char),
	                       MYF(MY_WME));
      end= my_stpcpy(query, "SET GLOBAL validate_password_policy = ");
      *end++ = '\'';
      end+= mysql_real_escape_string_quote(&mysql, end, strength,
                                           (ulong)strength_length, '\'');
      *end++ = '\'';
      if (!execute_query((const char **) &query,(unsigned int) (end-query)))
	DBUG_PRINT("info", ("query success!"));
      my_free(query);
    }
    else
      fprintf(stdout, "The password validation plugin is not available. "
	              "Proceeding with the further steps without the plugin.\n");
  }
  return(plugin_set);
}

/**
  Checks the password strength and displays it to the user.

  @param password_string    Password string whose strength
			    is to be estimated
*/
void estimate_password_strength(char *password_string)
{
  char *query, *end;
  size_t tmp= sizeof("SELECT validate_password_strength(") + 3;
  size_t password_length= strlen(password_string);
  /*
    query string needs memory which is atleast the length of initial part
    of query plus twice the size of variable being appended.
  */
  query= (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                           (password_length * 2 + tmp) * sizeof(char),
                           MYF(MY_WME));
  end= my_stpcpy(query, "SELECT validate_password_strength(");
  *end++ = '\'';
  end+= mysql_real_escape_string_quote(&mysql, end, password_string,
                                       (ulong)password_length, '\'');
  *end++ = '\'';
  *end++ = ')';
  if (!execute_query((const char **) &query,(unsigned int) (end-query)))
  {
    MYSQL_RES *result= mysql_store_result(&mysql);
    MYSQL_ROW row= mysql_fetch_row(result);
    printf("\nEstimated strength of the password: %s \n", row[0]);
    mysql_free_result(result);
  }
  my_free(query);
}


/**
  During rpm deployments the password expires immediately and needs to be
  renewed before the DBA can set the final password. This helper subroutine
  will use an active connection to set a password.

  @param mysql The MYSQL handle
  @param password A password character string

  Function might fail with an error message which can be retrieved using
  mysql_error(mysql)

  @return Success or failure
    @retval TRUE success
    @retval FALSE failure
*/

my_bool mysql_set_password(MYSQL *mysql, char *password)
{
  size_t password_len= strlen(password);
  char *query, *end;
  query= (char *)my_malloc(PSI_NOT_INSTRUMENTED, password_len+50, MYF(MY_WME));
  end= my_stpmov(query, "SET PASSWORD=");
  *end++ = '\'';
  end+= mysql_real_escape_string_quote(mysql, end, password,
                                       (ulong)password_len, '\'');
  *end++ = '\'';
  if (mysql_real_query(mysql, query, (ulong) (end - query)))
  {
    my_free(query);
    return FALSE;
  }

  my_free(query);
  return TRUE;
}


/**
  Expires the password for all users if executed with sufficient
  privileges. This is primarily used as a helper function during rpm
  deployments.

  @param mysql The MYSQL handle
  @param user The user name of the expired account
  @param host The host name of the expired account

  Function might fail with an error message which can be retrieved using
  mysql_error(mysql)

  @return Success or failure
    @retval TRUE success
    @retval FALSE failure
*/

my_bool mysql_expire_password(MYSQL *mysql)
{
  char sql[]= "UPDATE mysql.user SET password_expired= 'Y'";
  size_t sql_len= strlen(sql);
  if (mysql_real_query(mysql, sql, (ulong)sql_len))
    return FALSE;

  return TRUE;
}


/**
  Sets the user password with the string provided during the flow
  of the method. It checks for the strength of the password before
  changing it and displays the same to the user. The user can decide
  if he wants to continue with the password, or provide a new one,
  depending on the strength displayed.

  @param    plugin_set   1 if validate_password plugin is set and
                         0 if it is not.
*/

static void set_opt_user_password(int plugin_set)
{
  char *password1= 0, *password2= 0;
  int reply= 0;

  for(;;)
  {
    if (password1)
    {
      my_free(password1);
      password1= NULL;
    }
    if (password2)
    {
      my_free(password2);
      password2= NULL;
    }

    password1= get_tty_password("\nNew password: ");

    if (password1[0] == '\0')
    {
      fprintf(stdout, "Sorry, you can't use an empty password here.\n");
      continue;
    }

    password2= get_tty_password("\nRe-enter new password: ");

    if (strcmp(password1, password2))
    {
      fprintf(stdout, "Sorry, passwords do not match.\n");
      continue;
    }

    if (plugin_set == 1)
    {
      estimate_password_strength(password1);
      reply= get_response((const char *) "Do you wish to continue with the "
	                                 "password provided?(Press y|Y for "
					 "Yes, any other key for No) : ");
    }

    size_t pass_length= strlen(password1);

    if ((!plugin_set) || (reply == (int) 'y' || reply == (int) 'Y'))
    {
      char *query= NULL, *end;
      int tmp= sizeof("SET PASSWORD=") + 3;
      /*
	query string needs memory which is atleast the length of initial part
	of query plus twice the size of variable being appended.
      */
      query= (char *)my_malloc(PSI_NOT_INSTRUMENTED,
	                       (pass_length*2 + tmp)*sizeof(char), MYF(MY_WME));
      end= my_stpcpy(query, "SET PASSWORD=");
      *end++ = '\'';
      end+= mysql_real_escape_string_quote(&mysql, end, password1,
                                           (ulong)pass_length, '\'');
      *end++ = '\'';
      my_free(password1);
      my_free(password2);
      password1= NULL;
      password2= NULL;
      if (!execute_query((const char **)&query,(unsigned int) (end-query)))
      {
	my_free(query);
        break;
      }
      else
	fprintf(stdout, " ... Failed! Error: %s\n", mysql_error(&mysql));
    }
  }
}

/**
  Takes the opt_user's password as an input from the user and checks its validity
  by trying to connect to the server with it. The connection to the server
  is opened in this function.

  @return    Returns 1 if a password already exists and 0 if it doesn't.
*/
int get_opt_user_password()
{
  my_bool using_temporary_password= FALSE;
  int res;

  if (!password_provided)
  {
    /*
      No password is provided on the command line. Attempt to connect using
      a blank password.
    */
    MYSQL *con= mysql_real_connect(&mysql, opt_host, opt_user, "", "",
                            opt_port, opt_socket, 0);
    if (con != NULL || mysql_errno(&mysql) == ER_MUST_CHANGE_PASSWORD_LOGIN)
    {
      fprintf(stdout,"Connecting to MySQL using a blank password.\n");
      my_free(password);
      password= 0;
      mysql_close(con);
    }
    else
    {
      /*
        No password is provided and we cannot connect with a blank password.
        Assume there is an ongoing deployment running and attempt to locate
        the temporary password file.
      */
      char *temp_pass;
      if (find_temporary_password(&temp_pass) == TRUE)
      {
        my_free(password);
        password= temp_pass;
        using_temporary_password= TRUE;
      }
      else
      {
        char prompt[128];
        my_snprintf(prompt, sizeof(prompt) - 1,
                    "Enter password for user %s: ", opt_user);
        // Request password from user
        password= get_tty_password(prompt);
      }
    }
    init_connection_options(&mysql);
  } // if !password_provided

  /*
    A password candidate is identified. Use it to establish a connection.
  */
  if (!mysql_real_connect(&mysql, opt_host, opt_user,
			  password, "", opt_port, opt_socket, 0))
  {
    if (mysql_errno(&mysql) == ER_MUST_CHANGE_PASSWORD_LOGIN)
    {
      bool can= TRUE;
      init_connection_options(&mysql);
      mysql_options(&mysql, MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, &can);
      if (!mysql_real_connect(&mysql, opt_host, opt_user,
                              password, "", opt_port, opt_socket, 0))
      {
	fprintf(stdout, "Error: %s\n", mysql_error(&mysql));
	free_resources();
	exit(1);
      }

      /*
        Password worked but has expired. If this happens during a silent
        deployment using the rpm package system we cannot stop and ask
        for a password. Instead we just renew the previous password and set
        it to expire.
      */
      if (using_temporary_password)
      {
        if (!mysql_set_password(&mysql, password))
        {
          fprintf(stdout, "... Failed! Error: %s\n",
                  mysql_error(&mysql));
          free_resources();
          exit(1);
        }
        g_expire_password_on_exit= TRUE;
      }
      else
      {
        /*
          This path is only executed if no temporary password can be found and
          should only happen when manual interaction is possible.
        */
        fprintf(stdout, "\nThe existing password for the user account %s has "
	                "expired. Please set a new password.\n",
                        opt_user);
        set_opt_user_password(0);
      }
    }
    else
    {
      fprintf(stdout, "Error: %s\n", mysql_error(&mysql));
      free_resources();
      exit(1);
    }
  }
  res= (password && password[0] != '\0') ? 1 : 0;
  return(res);
}


/**
  Takes the user and the host from result set and drops those users.

  @param result    The result set from which rows are to be fetched.
*/
void drop_users(MYSQL_RES *result)
{
  MYSQL_ROW row;
  char *user_tmp, *host_tmp;
  while ((row= mysql_fetch_row(result)))
  {
    char *query, *end;
    size_t user_length, host_length;
    int tmp= sizeof("DROP USER ")+5;
    user_tmp= row[0];
    host_tmp= row[1];
    user_length= strlen(user_tmp);
    host_length= strlen(host_tmp);
    /*
      query string needs memory which is atleast the length of initial part
      of query plus twice the size of variable being appended.
    */
    query= (char *)my_malloc(PSI_NOT_INSTRUMENTED,
	                     ((user_length + host_length)*2 + tmp) *
	                     sizeof(char), MYF(MY_WME));
    end= my_stpcpy(query, "DROP USER ");
    *end++ = '\'';
    end+= mysql_real_escape_string_quote(&mysql, end, user_tmp,
                                         (ulong)user_length, '\'');
    *end++ = '\'';
    *end++ = '@';
    *end++ = '\'';
    end+= mysql_real_escape_string_quote(&mysql, end, host_tmp,
                                         (ulong)host_length, '\'');
    *end++ = '\'';
    if (!execute_query((const char **) &query, (unsigned int) (end-query)))
      DBUG_PRINT("info", ("query success!"));
    my_free(query);
  }
}

/**
  Removes all the anonymous users for better security.
*/
void remove_anonymous_users()
{
  int reply;
  reply= get_response((const char *) "By default, a MySQL installation has an "
				     "anonymous user,\nallowing anyone to log "
				     "into MySQL without having to have\na user "
				     "account created for them. This is intended "
				     "only for\ntesting, and to make the "
				     "installation go a bit smoother.\nYou should "
				     "remove them before moving into a production\n"
				     "environment.\n\nRemove anonymous users? "
				     "(Press y|Y for Yes, any other key for No) : ", 'y');

  if (reply == (int) 'y' || reply == (int) 'Y')
  {
    const char *query;
    query= "SELECT USER, HOST FROM mysql.user WHERE USER=''";
    if (!execute_query(&query, strlen(query)))
      DBUG_PRINT("info", ("query success!"));
    MYSQL_RES *result= mysql_store_result(&mysql);
    if (result)
      drop_users(result);
    mysql_free_result(result);
    fprintf(stdout, "Success.\n\n");
  }
  else
    fprintf(stdout, "\n ... skipping.\n\n");
}


/**
  Drops all the root users with a remote host.
*/
void remove_remote_root()
{
  int reply;
  reply= get_response((const char *) "\nNormally, root should only be "
                                     "allowed to connect from\n'localhost'. "
				     "This ensures that someone cannot guess at"
				     "\nthe root password from the network.\n\n"
				     "Disallow root login remotely? (Press y|Y "
				     "for Yes, any other key for No) : ", 'y');
  if (reply == (int) 'y' || reply == (int) 'Y')
  {
    const char *query;
    query= "SELECT USER, HOST FROM mysql.user WHERE USER='root' "
	   "AND HOST NOT IN ('localhost', '127.0.0.1', '::1')";
    if (!execute_query(&query, strlen(query)))
      DBUG_PRINT("info", ("query success!"));
    MYSQL_RES *result= mysql_store_result(&mysql);
    if (result)
      drop_users(result);
    mysql_free_result(result);
    fprintf(stdout, "Success.\n\n");
  }
  else
    fprintf(stdout, "\n ... skipping.\n");
}

/**
  Removes test database and deletes the rows corresponding to them
  from mysql.db table.
*/
void remove_test_database()
{
  int reply;
  reply= get_response((const char *) "By default, MySQL comes with a database "
                                     "named 'test' that\nanyone can access. "
				     "This is also intended only for testing,\n"
				     "and should be removed before moving into "
				     "a production\nenvironment.\n\n\nRemove "
				     "test database and access to it? (Press "
				     "y|Y for Yes, any other key for No) : ", 'y');
  if (reply == (int) 'y' || reply == (int) 'Y')
  {
    execute_query_with_message((const char *) "DROP DATABASE IF EXISTS test",
			       (const char *) " - Dropping test database...\n");

    execute_query_with_message((const char *) "DELETE FROM mysql.db WHERE "
	                                      "Db='test' OR Db='test\\_%'",
			       (const char *) " - Removing privileges on test "
			                      "database...\n");
  }
  else
    fprintf(stdout, "\n ... skipping.\n");
}

/**
  Refreshes the in-memory details through
  FLUSH PRIVILEGES.
*/
void reload_privilege_tables()
{
  int reply;
  reply= get_response((const char *) "Reloading the privilege tables will "
                                     "ensure that all changes\nmade so far "
				     "will take effect immediately.\n\nReload "
				     "privilege tables now? (Press y|Y for "
				     "Yes, any other key for No) : ", 'y');
  if (reply == (int) 'y' || reply == (int) 'Y')
  {
    execute_query_with_message((const char *) "FLUSH PRIVILEGES", NULL);
  }
  else
    fprintf(stdout, "\n ... skipping.\n");
}


/**
  Attempt to retrieve a password from the temporary password file
  '.mysql_secret'.
 @param p[out] A pointer to a password in a newly allocated buffer or null
 @returns true if the password was successfully retrieved.
*/

my_bool find_temporary_password(char **p)
{
  const char *root_path= "/root";
  const char *password_file_name= "/.mysql_secret";
  *p= NULL;
  const char *home= getenv("HOME");
  if (home == NULL)
    home= root_path;

  size_t home_len= strlen(home);
  size_t path_len= home_len + strlen(password_file_name)+1;
  char *path= (char *)malloc(path_len);
  memset(path, 0, path_len);
  
  strcat(path, home);
  strcat(path, password_file_name);
  FILE *fp= fopen(path, "r");
  if (fp == NULL)
  {
    free(path);
    return FALSE;
  }

  /*
    The format of the password file is
    ['#'][bytes]['\n']['password bytes']['\n']|[EOF])
  */
  char header[256];
  char password[64];
  size_t password_len=0;
  /* Read header and skip it */
  if (fgets(&header[0], sizeof(header), fp) == NULL || header[0] != '#')
    goto error;

  /* Read password */
  if (fgets(&password[0], sizeof(password), fp) == NULL)
    goto error;

  /* Remove terminating newline character if it exists */
  password_len= strlen(&password[0]);
  if (password[password_len - 1] == '\n')
    password[password_len - 1] = '\0';

  *p= my_strdup(PSI_NOT_INSTRUMENTED,
		&password[0], MYF(MY_FAE));
  fprintf(stdout, "Connecting to MySQL server using password in '%s'\n",path);
  
  free(path);
  return TRUE;

  error:
    fprintf(stdout, "The password file '%s' is corrupt! Skipping.\n", path);
    if (path)
      free(path);
    return FALSE;
}


int main(int argc,char *argv[])
{
  int reply;
  int rc;
  int hadpass, plugin_set= 0;

  MY_INIT(argv[0]);
  DBUG_ENTER("main");
  DBUG_PROCESS(argv[0]);
  if (mysql_init(&mysql) == NULL)
  {
    printf("... Failed to initialize the MySQL client framework.\n");
    exit(1);
  }

#ifdef __WIN__
  /* Convert command line parameters from UTF16LE to UTF8MB4. */
  my_win_translate_command_line_args(&my_charset_utf8mb4_bin, &argc, &argv);
#endif

  my_getopt_use_args_separator= TRUE;
  if (load_defaults("my", load_default_groups, &argc, &argv))
  {
    my_end(0);
    free_resources();
    exit(1);
  }

  defaults_argv= argv;
  my_getopt_use_args_separator= FALSE;

  if ((rc= my_handle_options(&argc, &argv, my_connection_options,
                             my_arguments_get_one_option, NULL, TRUE)))
  {
    DBUG_ASSERT(0);
  }

  init_connection_options(&mysql);

  fprintf(stdout, "\nSecuring the MySQL server deployment.\n\n");

  hadpass= get_opt_user_password();

  if (!validate_password_exists())
    plugin_set= install_password_validation_plugin();
  else
  {
    fprintf(stdout, "The 'validate_password' plugin is installed on the server.\n"
	            "The subsequent steps will run with the existing "
		    "configuration\nof the plugin.\n");
    plugin_set= 1;
  }


  if (!hadpass)
  {
    fprintf(stdout, "Please set the password for %s here.\n", opt_user);
    set_opt_user_password(plugin_set);
  }
  else if (opt_use_default == FALSE)
  {
    char prompt[256];
    fprintf(stdout, "Using existing password for %s.\n", opt_user);

    if (plugin_set == 1)
      estimate_password_strength(password);

    my_snprintf(prompt, sizeof(prompt) - 1, 
                "Change the password for %s ? ((Press y|Y "
                "for Yes, any other key for No) : ", opt_user);
    reply= get_response(prompt, 'n');

    if (reply == (int) 'y' || reply == (int) 'Y')
      set_opt_user_password(plugin_set);
    else
      fprintf(stdout, "\n ... skipping.\n");
  }

  //Remove anonymous users
  remove_anonymous_users();

  //Disallow remote root login
  remove_remote_root();

  //Remove test database
  remove_test_database();

  /*
    During an unattended rpm deployment a temporary password is created and
    stored in a file by mysql_install_db. This program use this password to
    perform security configurations after the bootstrap phase but it needs to
    be marked for expiration upon exit so the DBA will remember to set a new
    one.
  */
  if (g_expire_password_on_exit == TRUE)
  {
    if (mysql_expire_password(&mysql) == FALSE)
    {
      fprintf(stdout, "... Failed to expire password!\n"
                      "** Please consult the MySQL server documentation. **\n"
                      "Error: %s\n",
              mysql_error(&mysql));
      // Reload privilege tables before exiting
      reload_privilege_tables();
      free_resources();
      exit(1);
    }
  }

  //Reload privilege tables
  reload_privilege_tables();

  fprintf(stdout, "All done! \n");
  free_resources();
return 0;
}
 
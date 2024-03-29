/*
   Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
  Return error-text for NDB error messages in same
  fashion as "perror --ndb <error>"
*/

#include "ndb_global.h"
#include "ndb_opts.h"

#include "ndbapi/ndberror.h"
#include "mgmapi/ndbd_exit_codes.h"
#include "mgmapi/mgmapi_error.h"

static my_bool opt_verbose;
static my_bool opt_silent; // Overrides verbose and sets it to 0

static struct my_option my_long_options[] =
{
  {"help", '?', "Displays this help and exits.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
  {"ndb", NDB_OPT_NOSHORT,
   "For command line compatibility with 'perror --ndb', ignored.", 0,
   0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0 },
  {"silent", 's', "Only print the error message.", &opt_silent,
   &opt_silent, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"verbose", 'v', "Print error code and message (default).", &opt_verbose,
   &opt_verbose, 0, GET_BOOL, NO_ARG, 1, 0, 0, 0, 0, 0},
  {"version", 'V', "Displays version information and exits.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};

const char *load_default_groups[] = { 0 };

static void short_usage_sub(void)
{
  ndb_short_usage_sub("[ERRORCODE [ERRORCODE...]]");
}

static void usage()
{
  ndb_usage(short_usage_sub, load_default_groups, my_long_options);
}


static
int mgmapi_error_string(int err_no, char *str, int size)
{
  for (int i = 0; i < ndb_mgm_noOfErrorMsgs; i++)
  {
    if ((int)ndb_mgm_error_msgs[i].code == err_no)
    {
      my_snprintf(str, size - 1, "%s", ndb_mgm_error_msgs[i].msg);
      str[size - 1] = '\0';
      return 1; // Found a message
    }
  }
  return -1;
}


// Forward declare function from ndbd_exit_codes.cc which is not
// declared in any header
extern "C" int ndbd_exit_string(int err_no, char *str, unsigned int size);


int main(int argc, char** argv)
{
  NDB_INIT(argv[0]);

  ndb_opt_set_usage_funcs(short_usage_sub, usage);
  ndb_load_defaults(NULL, load_default_groups, &argc, &argv);

  if (handle_options(&argc, &argv, my_long_options,
                     ndb_std_get_one_option) != 0)
    exit(255);

  if (opt_silent)
  {
    // --silent overrides any verbose setting
    opt_verbose = 0;
  }

  if (!argc)
  {
    usage();
    exit(1);
  }

  int error = 0;
  for ( ; argc-- > 0 ; argv++)
  {
    int code=atoi(*argv);

    char error_string[1024];
    if ((ndb_error_string(code, error_string, sizeof(error_string)) > 0) ||
        (ndbd_exit_string(code, error_string, sizeof(error_string)) > 0) ||
        (mgmapi_error_string(code, error_string, sizeof(error_string)) > 0))
    {
      if (opt_verbose)
        printf("NDB error code %3d: %s\n", code, error_string);
      else
        puts(error_string);
    }
    else
    {
      fprintf(stderr, "Illegal ndb error code: %d\n", code);
      error= 1;
    }
  }
      
  exit(error);
  return error;
}
 
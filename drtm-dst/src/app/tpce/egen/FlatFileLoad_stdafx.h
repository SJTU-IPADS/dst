/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a part of a
 * benchmark specification maintained by the TPC.
 *
 * The TPC reserves all right, title, and interest to the Work as provided
 * under U.S. and international laws, including without limitation all patent
 * and trademark rights therein.
 *
 * No Warranty
 *
 * 1.1 TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE INFORMATION
 *     CONTAINED HEREIN IS PROVIDED "AS IS" AND WITH ALL FAULTS, AND THE
 *     AUTHORS AND DEVELOPERS OF THE WORK HEREBY DISCLAIM ALL OTHER
 *     WARRANTIES AND CONDITIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
 *     INCLUDING, BUT NOT LIMITED TO, ANY (IF ANY) IMPLIED WARRANTIES,
 *     DUTIES OR CONDITIONS OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
 *     PURPOSE, OF ACCURACY OR COMPLETENESS OF RESPONSES, OF RESULTS, OF
 *     WORKMANLIKE EFFORT, OF LACK OF VIRUSES, AND OF LACK OF NEGLIGENCE.
 *     ALSO, THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,
 *     QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR NON-INFRINGEMENT
 *     WITH REGARD TO THE WORK.
 * 1.2 IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THE WORK BE LIABLE TO
 *     ANY OTHER PARTY FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO THE
 *     COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST PROFITS, LOSS
 *     OF USE, LOSS OF DATA, OR ANY INCIDENTAL, CONSEQUENTIAL, DIRECT,
 *     INDIRECT, OR SPECIAL DAMAGES WHETHER UNDER CONTRACT, TORT, WARRANTY,
 *     OR OTHERWISE, ARISING IN ANY WAY OUT OF THIS OR ANY OTHER AGREEMENT
 *     RELATING TO THE WORK, WHETHER OR NOT SUCH AUTHOR OR DEVELOPER HAD
 *     ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Contributors
 * - Sergey Vasilevskiy
 */

#ifndef FLAT_FILE_LOAD_STDAFX_H
#define FLAT_FILE_LOAD_STDAFX_H

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#include <iostream>
#include <fstream>
#include <cstdio>

#include "EGenUtilities_stdafx.h"
#include "Table_Defs.h"
#include "EGenBaseLoader_stdafx.h"
#include "FlatFileLoader.h"
#include "FlatAccountPermissionLoad.h"
#include "FlatAddressLoad.h"
#include "FlatBrokerLoad.h"
#include "FlatCashTransactionLoad.h"
#include "FlatChargeLoad.h"
#include "FlatCommissionRateLoad.h"
#include "FlatCompanyLoad.h"
#include "FlatCompanyCompetitorLoad.h"
#include "FlatCustomerLoad.h"
#include "FlatCustomerAccountLoad.h"
#include "FlatCustomerTaxrateLoad.h"
#include "FlatDailyMarketLoad.h"
#include "FlatExchangeLoad.h"
#include "FlatFinancialLoad.h"
#include "FlatHoldingLoad.h"
#include "FlatHoldingHistoryLoad.h"
#include "FlatHoldingSummaryLoad.h"
#include "FlatIndustryLoad.h"
#include "FlatLastTradeLoad.h"
#include "FlatNewsItemLoad.h"
#include "FlatNewsXRefLoad.h"
#include "FlatSectorLoad.h"
#include "FlatSecurityLoad.h"
#include "FlatSettlementLoad.h"
#include "FlatStatusTypeLoad.h"
#include "FlatTaxrateLoad.h"
#include "FlatTradeLoad.h"
#include "FlatTradeHistoryLoad.h"
#include "FlatTradeRequestLoad.h"
#include "FlatTradeTypeLoad.h"
#include "FlatWatchItemLoad.h"
#include "FlatWatchListLoad.h"
#include "FlatZipCodeLoad.h"
#include "FlatLoaderFactory.h"

#endif  // #ifndef FLAT_FILE_LOAD_STDAFX_H
 
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
 * - Doug Johnson
 */

/******************************************************************************
*   Description:        This class provides price board functionality for the
*                       MEE. This allows for the lookup of any security's price
*                       at any point in time.
******************************************************************************/

#ifndef MEE_PRICE_BOARD_H
#define MEE_PRICE_BOARD_H

#include "EGenUtilities_stdafx.h"
#include "EGenTables_stdafx.h"
#include "MEESecurity.h"

namespace TPCE
{

class CMEEPriceBoard
{

private:

    // Mean delay between Pending and Submission times
    // for an immediatelly triggered (in-the-money) limit order.
    //
    double              m_fMeanInTheMoneySubmissionDelay;
    CMEESecurity        m_Security;
    CSecurityFile*      m_pSecurityFile;

public:
    TIdent              m_iNumberOfSecurities;

    CMEEPriceBoard( INT32           TradingTimeSoFar,
                    CDateTime*      pBaseTime,
                    CDateTime*      pCurrentTime,
                    const CInputFiles& inputFiles
                    );
    ~CMEEPriceBoard( void );

    void    GetSymbol(  TIdent  SecurityIndex,
                        char*   szOutput,       // output buffer
                        size_t  iOutputLen);    // size of the output buffer (including null));

    CMoney  GetMinPrice();

    CMoney  GetMaxPrice();

    CMoney  GetCurrentPrice( TIdent SecurityIndex );
    CMoney  GetCurrentPrice( char* pSecuritySymbol );

    CMoney  CalculatePrice( char* pSecuritySymbol, double fTime );

    double  GetSubmissionTime(
                                char*           pSecuritySymbol,
                                double          fPendingTime,
                                CMoney          fLimitPrice,
                                eTradeTypeID    TradeType
                                );
    double  GetSubmissionTime(
                                TIdent          SecurityIndex,
                                double          fPendingTime,
                                CMoney          fLimitPrice,
                                eTradeTypeID    TradeType
                                );
    double  GetCompletionTime(
                                TIdent      SecurityIndex,
                                double      fSubmissionTime,
                                CMoney*     pCompletionPrice    // output param
                            );
};

}   // namespace TPCE

#endif //MEE_PRICE_BOARD_H
 
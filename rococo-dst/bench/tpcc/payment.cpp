
#include "all.h"

namespace rococo {

void TpccPiece::reg_payment() {
    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_0,    // piece 0, Ri & W warehouse
            DF_NO) { // immediately read
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 2);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_0);
        i32 oi = 0;
        // ############################################################
        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_WAREHOUSE),
                input[0], output_size, header.pid).next();

        // ############################################################
        TPL_KISS (
                mdb::column_lock_t(r, 1, ALock::RLOCK),
                mdb::column_lock_t(r, 2, ALock::RLOCK),
                mdb::column_lock_t(r, 3, ALock::RLOCK),
                mdb::column_lock_t(r, 4, ALock::RLOCK),
                mdb::column_lock_t(r, 5, ALock::RLOCK),
                mdb::column_lock_t(r, 6, ALock::RLOCK)
        );
        // ############################################################

        // R warehouse
        dtxn->read_column(r, 1, &output[oi++]);  // 0 ==> w_name
        dtxn->read_column(r, 2, &output[oi++]);  // 1 ==> w_street_1
        dtxn->read_column(r, 3, &output[oi++]);  // 2 ==> w_street_2
        dtxn->read_column(r, 4, &output[oi++]);  // 3 ==> w_city
        dtxn->read_column(r, 5, &output[oi++]);  // 4 ==> w_state
        dtxn->read_column(r, 6, &output[oi++]);  // 5 ==> w_zip

        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d end", TPCC_PAYMENT_0);
        // ############################################################
    } END_PIE

    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_1,    // piece 1, Ri district
            DF_NO) { // immediately read
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 2);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_1);
        // ############################################################
        i32 oi = 0;
        Value buf;
        mdb::MultiBlob mb(2);
        mb[0] = input[1].get_blob();
        mb[1] = input[0].get_blob();
        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_DISTRICT), mb,
                output_size, header.pid).next();

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 2, ALock::RLOCK),
                mdb::column_lock_t(r, 3, ALock::RLOCK),
                mdb::column_lock_t(r, 4, ALock::RLOCK),
                mdb::column_lock_t(r, 5, ALock::RLOCK),
                mdb::column_lock_t(r, 6, ALock::RLOCK),
                mdb::column_lock_t(r, 7, ALock::RLOCK)
        );
        // ############################################################

        // R district
        dtxn->read_column(r, 2, &output[oi++]); // output[0] ==> d_name
        dtxn->read_column(r, 3, &output[oi++]); // 1 ==> d_street_1
        dtxn->read_column(r, 4, &output[oi++]); // 2 ==> d_street_2
        dtxn->read_column(r, 5, &output[oi++]); // 3 ==> d_city
        dtxn->read_column(r, 6, &output[oi++]); // 4 ==> d_state
        dtxn->read_column(r, 7, &output[oi++]); // 5 ==> d_zip

        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d end", TPCC_PAYMENT_1);
        // ############################################################
    } END_PIE


    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_2,    // piece 1, Ri & W district
            DF_REAL) {
        // ############################################################
        verify(input_size == 3);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_2);
        i32 output_index = 0;
        // ############################################################
        
        Value buf;
        mdb::Row *r = NULL;
        mdb::MultiBlob mb(2);
        //cell_locator_t cl(TPCC_TB_DISTRICT, 2);
        mb[0] = input[1].get_blob();
        mb[1] = input[0].get_blob();
        if (!(IS_MODE_RCC || IS_MODE_RO6) 
                || ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc || rcc start request
            r = dtxn->query(dtxn->get_table(TPCC_TB_DISTRICT), mb,
                    output_size, header.pid).next();
            verify(r->schema_ != nullptr);
        }

        TPL_KISS(mdb::column_lock_t(r, 9, ALock::WLOCK));

        RCC_KISS(r, 9, false);
        RCC_SAVE_ROW(r, TPCC_PAYMENT_2);
        RCC_PHASE1_RET;
        RCC_LOAD_ROW(r, TPCC_PAYMENT_2);

        verify(r->schema_ != nullptr);
        dtxn->read_column(r, 9, &buf);

        // W district
        buf.set_double(buf.get_double() + input[2].get_double());
        dtxn->write_column(r, 9, buf);

        // ############################################################
        verify(*output_size >= output_index);
        *output_size = output_index;
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d end", TPCC_PAYMENT_2);
        // ############################################################
    } END_PIE


    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_3,    // piece 2, R customer secondary index, c_last -> c_id
            DF_NO) {
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 3);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_3);
        // ############################################################

        TPL_KISS_NONE;

        i32 output_index = 0;
        mdb::MultiBlob mbl(3), mbh(3);
        mbl[0] = input[2].get_blob();
        mbh[0] = input[2].get_blob();
        mbl[1] = input[1].get_blob();
        mbh[1] = input[1].get_blob();
        Value c_id_low(std::numeric_limits<i32>::min()), c_id_high(std::numeric_limits<i32>::max());
        mbl[2] = c_id_low.get_blob();
        mbh[2] = c_id_high.get_blob();
        c_last_id_t key_low(input[0].get_str(), mbl, &g_c_last_schema);
        c_last_id_t key_high(input[0].get_str(), mbh, &g_c_last_schema);
        std::multimap<c_last_id_t, rrr::i32>::iterator it, it_low, it_high, it_mid;
        bool inc = false, mid_set = false;
        it_low = g_c_last2id.lower_bound(key_low);
        it_high = g_c_last2id.upper_bound(key_high);
        int n_c = 0;
        for (it = it_low; it != it_high; it++) {
            n_c++;
            if (mid_set) if (inc) {
                it_mid++;
                inc = false;
            }
            else
                inc = true;
            else {
                mid_set = true;
                it_mid = it;
            }
        }
        Log::debug("w_id: %d, d_id: %d, c_last: %s, num customer: %d", input[1].get_i32(), input[2].get_i32(), input[0].get_str().c_str(), n_c);
        verify(mid_set);
        output[output_index++] = Value(it_mid->second);

        // ############################################################
        verify(*output_size >= output_index);
        *output_size = output_index;
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d, end", TPCC_PAYMENT_3);
        // ############################################################
    } END_PIE

    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_4,    // piece 4, R & W customer
            DF_REAL) {
        // ############################################################
        verify(input_size == 6);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_4);
        i32 output_index = 0;
        // ############################################################
        mdb::Row *r = NULL;
        std::vector<Value> buf;

        mdb::MultiBlob mb(3);
        //cell_locator_t cl(TPCC_TB_CUSTOMER, 3);
        mb[0] = input[0].get_blob();
        mb[1] = input[2].get_blob();
        mb[2] = input[1].get_blob();
        // R customer
        if (!(IS_MODE_RCC || IS_MODE_RO6) || 
                ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc || rcc start request
            r = dtxn->query(dtxn->get_table(TPCC_TB_CUSTOMER), mb,
                    output_size, header.pid).next();
        }


        ALock::type_t lock_20_type = ALock::RLOCK;
        if (input[0].get_i32() % 10 == 0)
            lock_20_type = ALock::WLOCK;
        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 3, ALock::RLOCK),
                mdb::column_lock_t(r, 4, ALock::RLOCK),
                mdb::column_lock_t(r, 5, ALock::RLOCK),
                mdb::column_lock_t(r, 6, ALock::RLOCK),
                mdb::column_lock_t(r, 7, ALock::RLOCK),
                mdb::column_lock_t(r, 8, ALock::RLOCK),
                mdb::column_lock_t(r, 9, ALock::RLOCK),
                mdb::column_lock_t(r, 10, ALock::RLOCK),
                mdb::column_lock_t(r, 11, ALock::RLOCK),
                mdb::column_lock_t(r, 12, ALock::RLOCK),
                mdb::column_lock_t(r, 13, ALock::RLOCK),
                mdb::column_lock_t(r, 14, ALock::RLOCK),
                mdb::column_lock_t(r, 15, ALock::RLOCK),
                mdb::column_lock_t(r, 16, ALock::WLOCK),
                mdb::column_lock_t(r, 17, ALock::WLOCK),
                mdb::column_lock_t(r, 20, lock_20_type)
        )
        // ############################################################
        RCC_KISS(r, 16, false);
        RCC_KISS(r, 17, false);
        RCC_KISS(r, 20, false);
        RCC_SAVE_ROW(r, TPCC_PAYMENT_4);
        RCC_PHASE1_RET;
        RCC_LOAD_ROW(r, TPCC_PAYMENT_4);

        if (!dtxn->read_columns(r, std::vector<mdb::column_id_t>({
                3,  // c_first          buf[0]
                4,  // c_middle         buf[1]
                5,  // c_last           buf[2]
                6,  // c_street_1       buf[3]
                7,  // c_street_2       buf[4]
                8,  // c_city           buf[5]
                9,  // c_state          buf[6]
                10, // c_zip            buf[7]
                11, // c_phone          buf[8]
                12, // c_since          buf[9]
                13, // c_credit         buf[10]
                14, // c_credit_lim     buf[11]
                15, // c_discount       buf[12]
                16, // c_balance        buf[13]
                17, // c_ytd_payment    buf[14]
                20  // c_data           buf[15]
        }), &buf)) {
            *res = REJECT;
            *output_size = output_index;
            return;
        }

        // if c_credit == "BC" (bad) 10%
        // here we use c_id to pick up 10% instead of c_credit
        if (input[0].get_i32() % 10 == 0) {
            Value c_data((
                    to_string(input[0])
                            + to_string(input[2])
                            + to_string(input[1])
                            + to_string(input[5])
                            + to_string(input[4])
                            + to_string(input[3])
                            + buf[15].get_str()
            ).substr(0, 500));
            std::vector<mdb::column_id_t> col_ids({
                    16, // c_balance
                    17, // c_ytd_payment
                    20  // c_data
            });
            std::vector<Value> col_data({
                    Value(buf[13].get_double() - input[3].get_double()),
                    Value(buf[14].get_double() + input[3].get_double()),
                    c_data
            });
            dtxn->write_columns(r, col_ids, col_data);
        }
        else {
            std::vector<mdb::column_id_t> col_ids({
                    16, // c_balance
                    17  // c_ytd_payment
            });
            std::vector<Value> col_data({
                    Value(buf[13].get_double() - input[3].get_double()),
                    Value(buf[14].get_double() + input[3].get_double())
            });
            dtxn->write_columns(r, col_ids, col_data);
        }

        output[output_index++] = input[0];  // output[0]  =>  c_id
        output[output_index++] = buf[0];    // output[1]  =>  c_first
        output[output_index++] = buf[1];    // output[2]  =>  c_middle
        output[output_index++] = buf[2];    // output[3]  =>  c_last
        output[output_index++] = buf[3];    // output[4]  =>  c_street_1
        output[output_index++] = buf[4];    // output[5]  =>  c_street_2
        output[output_index++] = buf[5];    // output[6]  =>  c_city
        output[output_index++] = buf[6];    // output[7]  =>  c_state
        output[output_index++] = buf[7];    // output[8]  =>  c_zip
        output[output_index++] = buf[8];    // output[9]  =>  c_phone
        output[output_index++] = buf[9];    // output[10] =>  c_since
        output[output_index++] = buf[10];   // output[11] =>  c_credit
        output[output_index++] = buf[11];   // output[12] =>  c_credit_lim
        output[output_index++] = buf[12];   // output[13] =>  c_discount
        output[output_index++] = Value(buf[13].get_double() - input[3].get_double()); // output[14] =>  c_balance

        // ############################################################
        verify(*output_size >= output_index);
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d end", TPCC_PAYMENT_4);
        // ############################################################
        *output_size = output_index;
    } END_PIE

    BEGIN_PIE(TPCC_PAYMENT,      // txn
            TPCC_PAYMENT_5,    // piece 4, W histroy
            DF_REAL) {
        // ############################################################
        verify(input_size == 9);
        Log::debug("TPCC_PAYMENT, piece: %d", TPCC_PAYMENT_5);
        // ############################################################

        TPL_KISS_NONE;

        i32 output_index = 0;
        mdb::Txn *txn = DTxnMgr::get_sole_mgr()->get_mdb_txn(header);
        mdb::Table *tbl = txn->get_table(TPCC_TB_HISTORY);

        // insert history
        mdb::Row *r = NULL;
        if (!(IS_MODE_RCC || IS_MODE_RO6) || 
                ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc || rcc start request
        }

        RCC_PHASE1_RET;

        std::vector<Value> row_data(9);
        row_data[0] = input[0];             // h_key
        row_data[1] = input[5];             // h_c_id   =>  c_id
        row_data[2] = input[7];             // h_c_d_id =>  c_d_id
        row_data[3] = input[6];             // h_c_w_id =>  c_w_id
        row_data[4] = input[4];             // h_d_id   =>  d_id
        row_data[5] = input[3];             // h_d_w_id =>  d_w_id
        row_data[6] = Value(std::to_string(time(NULL)));    // h_date
        row_data[7] = input[8];             // h_amount =>  h_amount
        row_data[8] = Value(input[1].get_str() + "    " + input[2].get_str()); // d_data => w_name + 4spaces + d_name

        CREATE_ROW(tbl->schema(), row_data);

        txn->insert_row(tbl, r);

        // ############################################################
        verify(*output_size >= output_index);
        *res = SUCCESS;
        Log::debug("TPCC_PAYMENT, piece: %d end", TPCC_PAYMENT_5);
        // ############################################################
        *output_size = output_index;
    } END_PIE
}

} // namespace rococo
 
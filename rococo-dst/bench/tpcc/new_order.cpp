#include "all.h"

namespace rococo {

static uint32_t TXN_TYPE = TPCC_NEW_ORDER; 

void TpccPiece::reg_new_order() {
    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_0, // Ri & W district
            DF_NO) {
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 2);
        // ############################################################
        
        mdb::MultiBlob mb(2);
        mb[0] = input[1].get_blob();
        mb[1] = input[0].get_blob();
        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_DISTRICT), 
                mb, output_size, header.pid).next();

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 8, ALock::RLOCK),
                mdb::column_lock_t(r, 10, ALock::WLOCK)
        );

        if ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1) {
            ((RCCDTxn*)dtxn)->kiss(r, 10, true);
        }
        // ############################################################

        i32 oi = 0;
        Value buf;
        // R district
        dtxn->read_column(r, 8, &output[oi++]); // read d_tax
        dtxn->read_column(r, 10, &buf); // read d_next_o_id
        output[oi++] = buf;

        // W district
        buf.set_i32((i32)(buf.get_i32() + 1));
        dtxn->write_column(r, 10, buf); // read d_next_o_id, increament by 1

        // ############################################################
        verify(*output_size >= oi);
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_0);
        *res = SUCCESS;
        // ############################################################

        *output_size = oi;
        return;
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_1, // R warehouse
            DF_NO) {
        // ############################################################
        verify(input_size == 1);
        verify(row_map == NULL);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_1);
        // ############################################################

        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_WAREHOUSE), 
                input[0], output_size, header.pid).next();

        // ############################################################
        TPL_KISS(mdb::column_lock_t(r, 7, ALock::RLOCK));
        // ############################################################

        i32 oi = 0;
        // R warehouse
        dtxn->read_column(r, 7, &output[oi++]); // read w_tax

        // ############################################################
        verify(*output_size >= oi);
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_1);
        *res = SUCCESS;
        // ############################################################
        
        *output_size = oi;
        return;
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER, TPCC_NEW_ORDER_2, // R customer
            DF_NO //XXX either i or d is ok
    ) {
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 3);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_2);
        i32 oi = 0;
        // ############################################################

        mdb::MultiBlob mb(3);
        mb[0] = input[2].get_blob();
        mb[1] = input[1].get_blob();
        mb[2] = input[0].get_blob();
        auto table = dtxn->get_table(TPCC_TB_CUSTOMER);
        mdb::Row *r = dtxn->query(table, mb,
                output_size, header.pid).next();

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 5, ALock::RLOCK),
                mdb::column_lock_t(r, 13, ALock::RLOCK),
                mdb::column_lock_t(r, 15, ALock::RLOCK)
        );
        // ############################################################

        // R customer
        dtxn->read_column(r, 5, &output[oi++]);
        dtxn->read_column(r, 13, &output[oi++]);
        dtxn->read_column(r, 15, &output[oi++]);

        // ############################################################
        verify(*output_size >= oi);
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_2);
        *res = SUCCESS;
        // ############################################################
        
        *output_size = oi;
        return;
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_3, // W order
            DF_REAL) {
        // ############################################################
        verify(input_size == 7);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_3);
        i32 oi = 0;
        // ############################################################
        
        mdb::Table *tbl = dtxn->get_table(TPCC_TB_ORDER);

        mdb::MultiBlob mb(3);
        mb[0] = input[1].get_blob();
        mb[1] = input[2].get_blob();
        mb[2] = input[3].get_blob();

        mdb::Row *r = NULL;
        r = dtxn->query(dtxn->get_table(TPCC_TB_ORDER_C_ID_SECONDARY),
                    mb, false, header.pid).next();
        verify(r);
        verify(r->schema_);
        // ############################################################
        TPL_KISS(mdb::column_lock_t(r, 3, ALock::WLOCK));
        // ############################################################

        // W order
        if (!(IS_MODE_RCC || IS_MODE_RO6) ||
                ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc finish || rcc start request
            std::vector<Value> row_data({
                    input[1],   // o_d_id
                    input[2],   // o_w_id
                    input[0],   // o_id
                    input[3],   // o_c_id
                    Value(std::to_string(time(NULL))),  // o_entry_d
                    input[4],   // o_carrier_id
                    input[5],   // o_ol_cnt
                    input[6]    // o_all_local
            });

            CREATE_ROW(tbl->schema(), row_data);
        }

        verify(r->schema_);
        RCC_KISS(r, 0, false);
        RCC_KISS(r, 1, false);
        RCC_KISS(r, 2, false);
        RCC_KISS(r, 5, false);
        RCC_SAVE_ROW(r, TPCC_NEW_ORDER_3);
        RCC_PHASE1_RET; 
        RCC_LOAD_ROW(r, TPCC_NEW_ORDER_3);

        verify(r->schema_);
        dtxn->insert_row(tbl, r);

        // write TPCC_TB_ORDER_C_ID_SECONDARY
        //mdb::MultiBlob mb(3);
        //mb[0] = input[1].get_blob();
        //mb[1] = input[2].get_blob();
        //mb[2] = input[3].get_blob();
        r = dtxn->query(dtxn->get_table(TPCC_TB_ORDER_C_ID_SECONDARY),
                mb, true, header.pid).next();
        dtxn->write_column(r, 3, input[0]);

        // ############################################################
        verify(*output_size >= oi);
        *res = SUCCESS;
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_3);
        // ############################################################
        *output_size = oi;
        return;
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_4, // W new_order
            DF_REAL) {
        // ############################################################
        verify(input_size == 3);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_4);
        // ############################################################

        TPL_KISS_NONE;

        i32 oi = 0;
        mdb::Table *tbl = dtxn->get_table(TPCC_TB_NEW_ORDER);
        mdb::Row *r = NULL;

        // ############################################################
        // W new_order
        if (!(IS_MODE_RCC || IS_MODE_RO6) 
                || ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc || rcc start request
            std::vector<Value> row_data({
                    input[1],   // o_d_id
                    input[2],   // o_w_id
                    input[0],   // o_id
            });

            CREATE_ROW(tbl->schema(), row_data);
        }
        // ############################################################
        
        RCC_KISS(r, 0, false);
        RCC_KISS(r, 1, false);
        RCC_KISS(r, 2, false);
        RCC_SAVE_ROW(r, TPCC_NEW_ORDER_4);
        RCC_PHASE1_RET; 
        RCC_LOAD_ROW(r, TPCC_NEW_ORDER_4);

        dtxn->insert_row(tbl, r);
        // ############################################################
        verify(*output_size >= oi);
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_4);
        *res = SUCCESS;
        // ############################################################
        *output_size = oi;
        return;
    } END_PIE


    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_5, // Ri item
            DF_NO) {
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 1);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_5);
        // ############################################################
        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_ITEM), input[0],
                output_size, header.pid).next();

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 2, ALock::RLOCK),
                mdb::column_lock_t(r, 3, ALock::RLOCK),
                mdb::column_lock_t(r, 4, ALock::RLOCK)
        );
        // ############################################################

        i32 oi = 0;
        // Ri item
        dtxn->read_column(r, 2, &output[oi++]); // 0 ==> i_name
        dtxn->read_column(r, 3, &output[oi++]); // 1 ==> i_price
        dtxn->read_column(r, 4, &output[oi++]); // 2 ==> i_data

        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_5);
        *res = SUCCESS;
        // ############################################################
        return;
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_6, // Ri stock
            DF_NO) {
        // ############################################################
        verify(row_map == NULL);
        verify(input_size == 3);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_6);
        // ############################################################
       
        i32 oi = 0;
        Value buf;
        mdb::MultiBlob mb(2);
        mb[0] = input[0].get_blob();
        mb[1] = input[1].get_blob();
        mdb::Row *r = dtxn->query(dtxn->get_table(TPCC_TB_STOCK), mb,
                output_size, header.pid).next();
        verify(r->schema_);

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 3, ALock::RLOCK),
                mdb::column_lock_t(r, 16, ALock::RLOCK)
        );
        // ############################################################

        //i32 s_dist_col = 3 + input[2].get_i32();
        // Ri stock
        // FIXME compress all s_dist_xx into one column
        dtxn->read_column(r, 3, &output[oi++]); // 0 ==> s_dist_xx
        dtxn->read_column(r, 16, &output[oi++]); // 1 ==> s_data

        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        *res = SUCCESS;
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_6);
        // ############################################################
        return;
    } END_PIE


    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_7, // W stock
            DF_REAL) {
        verify(input_size == 4);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_7);
        i32 oi = 0;
        Value buf;
        mdb::Row *r = NULL;
        mdb::MultiBlob mb(2);
        mb[0] = input[0].get_blob();
        mb[1] = input[1].get_blob();

        if (!(IS_MODE_RCC || IS_MODE_RO6) 
                || ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // non-rcc || rcc start request
            r = dtxn->query(dtxn->get_table(TPCC_TB_STOCK), mb,
                    output_size, header.pid).next();
            verify(r->schema_);
        }

        // ############################################################
        TPL_KISS(
                mdb::column_lock_t(r, 2, ALock::WLOCK),
                mdb::column_lock_t(r, 13, ALock::WLOCK),
                mdb::column_lock_t(r, 14, ALock::WLOCK),
                mdb::column_lock_t(r, 15, ALock::WLOCK)
        );
        // ############################################################

        RCC_KISS(r, 2, false);
        RCC_KISS(r, 13, false);
        RCC_KISS(r, 14, false);
        RCC_KISS(r, 15, false);
        RCC_SAVE_ROW(r, TPCC_NEW_ORDER_7);
        RCC_PHASE1_RET; 
        RCC_LOAD_ROW(r, TPCC_NEW_ORDER_7);
        verify(r->schema_);

        // Ri stock
        i32 new_ol_quantity;
        dtxn->read_column(r, 2, &buf);
        new_ol_quantity = buf.get_i32() - input[2].get_i32();

        dtxn->read_column(r, 13, &buf);
        Value new_s_ytd(buf.get_i32() + input[2].get_i32());

        dtxn->read_column(r, 14, &buf);
        Value new_s_order_cnt((i32)(buf.get_i32() + 1));

        dtxn->read_column(r, 15, &buf);
        Value new_s_remote_cnt(buf.get_i32() + input[3].get_i32());

        if (new_ol_quantity < 10)
            new_ol_quantity += 91;
        Value new_ol_quantity_value(new_ol_quantity);

        if (!dtxn->write_columns(r, std::vector<mdb::column_id_t>({
                2,  // s_quantity
                13, // s_ytd
                14, // s_order_cnt
                15  // s_remote_cnt
        }), std::vector<Value>({
                new_ol_quantity_value,
                new_s_ytd,
                new_s_order_cnt,
                new_s_remote_cnt
        }))) {
            *res = REJECT;
            *output_size = oi;
            return;
        }
        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_7);
        // ############################################################
        *res = SUCCESS;
        return;
        
    } END_PIE

    BEGIN_PIE(TPCC_NEW_ORDER,
            TPCC_NEW_ORDER_8, // W order_line
            DF_REAL) {
        // ############################################################
        verify(input_size == 10);
        Log::debug("TPCC_NEW_ORDER, piece: %d", TPCC_NEW_ORDER_8);
        // ############################################################
        
        TPL_KISS_NONE;

        mdb::Table *tbl = dtxn->get_table(TPCC_TB_ORDER_LINE);
        mdb::Row *r = NULL;

        if (!(IS_MODE_RCC || IS_MODE_RO6) 
                || ((IS_MODE_RCC || IS_MODE_RO6) && IN_PHASE_1)) { 
            // 2pl finish || occ start || rcc start request
            std::vector<Value> input_buf(input, input + input_size);
            CREATE_ROW(tbl->schema(), input_buf);
        }

        RCC_KISS(r, 0, false);
        RCC_KISS(r, 1, false);
        RCC_KISS(r, 2, false);
        RCC_KISS(r, 3, false);
        RCC_KISS(r, 4, false);
        RCC_KISS(r, 6, false);
        RCC_KISS(r, 8, false);
        RCC_SAVE_ROW(r, TPCC_NEW_ORDER_8);
        RCC_PHASE1_RET;
        RCC_LOAD_ROW(r, TPCC_NEW_ORDER_8);
        
        i32 oi = 0;
        dtxn->insert_row(tbl, r);
        // ############################################################
        verify(*output_size >= oi);
        *output_size = oi;
        Log::debug("TPCC_NEW_ORDER, piece: %d end", TPCC_NEW_ORDER_8);
        // ############################################################
        *res = SUCCESS;
        return;
    } END_PIE
}
} // namespace rococo
 
#ifndef SHARDING_H_
#define SHARDING_H_

#include "all.h"

#include <map>
#include <string>
#include <vector>

namespace deptran {

class Config;
class MultiValue;

Value value_get_zero(Value::kind k);
Value value_get_n(Value::kind k, int n);
Value &operator ++(Value &lhs);
Value operator ++(Value &lhs, int);
Value random_value(Value::kind k);
Value value_rr_get_next(const std::string &s, Value::kind k, int max, int start = 0);

int init_index(std::map<unsigned int, std::pair<unsigned int, unsigned int> > &index);
int index_reverse_increase(std::map<unsigned int, std::pair<unsigned int, unsigned int> > &index);
int index_reverse_increase(std::map<unsigned int, std::pair<unsigned int, unsigned int> > &index, const std::vector<unsigned int> &bound_index);
int index_increase(std::map<unsigned int, std::pair<unsigned int, unsigned int> > &index);
int index_increase(std::map<unsigned int, std::pair<unsigned int, unsigned int> > &index, const std::vector<unsigned int> &bound_index);

//XXX hardcode for c_last secondary index
typedef struct c_last_id_t {
    std::string c_last;
    mdb::SortedMultiKey c_index_smk;

    c_last_id_t(const std::string &_c_last, const mdb::MultiBlob &mb, const mdb::Schema *schema) : c_last(_c_last), c_index_smk(mb, schema) {}

    bool operator<(const c_last_id_t &rhs) const {
        int ret = strcmp(c_last.c_str(), rhs.c_last.c_str());
        if (ret < 0)
            return true;
        else if (ret == 0) {
            if (c_index_smk < rhs.c_index_smk)
                return true;
        }
        return false;
    }
} c_last_id_t;
extern std::multimap<c_last_id_t, rrr::i32> g_c_last2id; // XXX
extern mdb::Schema g_c_last_schema;

class Sharding {
private:
    enum sharding_method_t {
        MODULUS,
        INT_MODULUS,
    };

    struct tb_info_t;

    typedef struct column_t {
        column_t(Value::kind _type, std::string _name, bool _is_primary, tb_info_t *_foreign_tb, column_t *_foreign) : type(_type), name(_name), is_primary(_is_primary), values(NULL) {
            foreign_tb = _foreign_tb;
            foreign = _foreign;
        }
        Value::kind type;
        std::string name;
        bool is_primary;
        column_t *foreign;
        tb_info_t *foreign_tb;
        std::vector<Value> *values;
    } column_t;

    typedef struct tb_info_t {
        tb_info_t() : sharding_method(MODULUS), num_site(0), site_id(NULL), num_records(0), populated(false) {}
        tb_info_t(std::string method, unsigned int ns = 0, unsigned int *sid = NULL, unsigned long long int _num_records = 0
                , mdb::symbol_t _symbol = mdb::TBL_UNSORTED
                ) : num_site(ns), site_id(sid), num_records(_num_records), populated(false)
                    , symbol(_symbol)
        {
            if (method == "modulus")
                sharding_method = MODULUS;
            else if (method == "int_modulus")
                sharding_method = INT_MODULUS;
            else
                sharding_method = MODULUS;
        }

        sharding_method_t sharding_method;
        unsigned int num_site;
        unsigned int *site_id;
        unsigned long long int num_records;
        bool populated;

        std::vector<column_t> columns;
        mdb::symbol_t symbol;
        std::string tb_name;
    } tb_info_t;

    std::map<std::string, tb_info_t> tb_info_;

    std::map<MultiValue, MultiValue> dist2sid_;
    std::map<MultiValue, MultiValue> stock2sid_;

    void insert_dist_mapping(const MultiValue &mv);

    MultiValue &dist_mapping(const MultiValue &mv);

    void insert_stock_mapping(const MultiValue &mv);

    MultiValue &stock_mapping(const MultiValue &mv);

    int get_site_id_from_tb(const std::string &tb_name, const MultiValue &key, unsigned int &site_id);

    int get_site_id_from_tb(const std::string &tb_name, std::vector<unsigned int> &site_id);

    int do_populate_table(const std::vector<std::string> &table_names, unsigned int sid);

    int do_tpcc_dist_partition_populate_table(const std::vector<std::string> &table_names, unsigned int sid);

    int do_tpcc_real_dist_partition_populate_table(const std::vector<std::string> &table_names, unsigned int sid);

    int do_tpcc_populate_table(const std::vector<std::string> &table_names, unsigned int sid);

    bool ready2populate(tb_info_t *tb_info);

    void release_foreign_values();

//    int get_site_id(const std::string &txn_name, unsigned int piece, const std::string &key, unsigned int &site_id);
//
//    int get_site_id(const std::string &txn_name, unsigned int piece, std::vector<unsigned int> &site_id);
//
    static unsigned int site_from_key(const MultiValue &key, const tb_info_t *tb_info);
    static unsigned int modulus(const MultiValue &key, unsigned int num_site, const unsigned int *site_id);
    static unsigned int int_modulus(const MultiValue &key, unsigned int num_site, const unsigned int *site_id);

    static Sharding *sharding_s;

    Sharding();

public:
    static int get_site_id(const char *tb_name, const Value &key, unsigned int &site_id);
    static int get_site_id(const std::string &tb_name, const Value &key, unsigned int &site_id);

    static int get_site_id(const char *tb_name, const MultiValue &key, unsigned int &site_id);
    static int get_site_id(const std::string &tb_name, const MultiValue &key, unsigned int &site_id);

    static int get_site_id(const char *tb_name, std::vector<unsigned int> &site_id);
    static int get_site_id(const std::string &tb_name, std::vector<unsigned int> &site_id);

    static int init_schema(const char *tb_name, mdb::Schema *schema, mdb::symbol_t *symbol);
    static int init_schema(const std::string &tb_name, mdb::Schema *schema, mdb::symbol_t *symbol);

    static int get_table_names(unsigned int sid, std::vector<std::string> &tables);
    static int get_number_rows(std::map<std::string, uint64_t> &table_map);

    static int populate_table(const std::vector<std::string> &table_names, unsigned int sid);

//    static int get_site_id(const char *txn_name, unsigned int piece, const char *key, unsigned int &site_id);
//    static int get_site_id(const std::string &txn_name, unsigned int piece, const std::string &key, unsigned int &site_id);
//
//    static int get_site_id(const char *txn_name, unsigned int piece, std::vector<unsigned int> &site_id);
//    static int get_site_id(const std::string &txn_name, unsigned int piece, std::vector<unsigned int> &site_id);
//

    ~Sharding();

friend class Config;
};

}

#endif
 
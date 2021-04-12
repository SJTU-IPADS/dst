#include "all.h"

namespace rococo {

ServerControlServiceImpl *ServerControlServiceImpl::scsi_s = NULL;

const char S_RES_KEY_N_SCC[]        =   "scc";
const char S_RES_KEY_N_ASK[]        =   "ask";
const char S_RES_KEY_START_GRAPH[]  =   "start_graph";
const char S_RES_KEY_COMMIT_GRAPH[] =   "commit_graph";
const char S_RES_KEY_ASK_GRAPH[]    =   "ask_graph";
//const char S_RES_KEY_CPU[]          =   "cpu";
//
//



const std::string ServerControlServiceImpl::STAT_SZ_SCC          =   "scc";
const std::string ServerControlServiceImpl::STAT_N_ASK           =   "ask";
const std::string ServerControlServiceImpl::STAT_SZ_GRAPH_START  =   "start_graph";
const std::string ServerControlServiceImpl::STAT_SZ_GRAPH_COMMIT =   "commit_graph";
const std::string ServerControlServiceImpl::STAT_SZ_GRAPH_ASK    =   "ask_graph";
const std::string ServerControlServiceImpl::STAT_RO6_SZ_VECTOR    =   "ack_start_vector";

void ServerControlServiceImpl::shutdown_wrapper(int sig) {
    if (scsi_s != NULL) {
        scsi_s->server_shutdown();
    }
}

void ServerControlServiceImpl::set_sig_handler() {
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = shutdown_wrapper;
    sigaction(SIGALRM, &sact, NULL);

    sig_handler_set_ = true;
}

void ServerControlServiceImpl::server_shutdown() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    status_ = SCS_STOP;
    //pthread_cond_broadcast(&status_cond_);
    status_cond_.bcast();
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ServerControlServiceImpl::server_ready(rrr::i32* res) {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    if (SCS_RUN == status_)
        *res = 1;
    else
        *res = 0;
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ServerControlServiceImpl::do_statistics(const char *key, 
        int64_t value_delta) {
    std::unordered_map<const char *, ValueTimesPair>::iterator it;
    //pthread_mutex_lock(&stat_m_);
    stat_m_.lock();
    it = statistics_.find(key);
    if (it == statistics_.end())
        statistics_[key] = (ValueTimesPair){value_delta, 1};
    else {
        it->second.value += value_delta;
        it->second.times++;
    }
    stat_m_.unlock();
    //pthread_mutex_unlock(&stat_m_);
}

void ServerControlServiceImpl::server_heart_beat() {
    if (!sig_handler_set_)
        set_sig_handler();
    alarm(timeout_);
}

void ServerControlServiceImpl::server_heart_beat_with_data(ServerResponse *res) {

    res->cpu_util = rrr::CPUInfo::cpu_stat();
    if (recorder_) {
        AvgStat r_cnt = recorder_->stat_cnt_.reset();
        AvgStat r_sz = recorder_->stat_sz_.reset();
        res->r_cnt_sum = r_cnt.sum_;
        res->r_cnt_num = r_cnt.n_stat_; // TODO [yangc] is there something wrong here?
        res->r_sz_sum = r_sz.sum_;
        res->r_sz_num = r_sz.n_stat_;
    }
    else {
        res->r_cnt_sum = 0;
        res->r_cnt_num = 0;
        res->r_sz_sum = 0;
        res->r_sz_num = 0;
    }
    if (!sig_handler_set_)
        set_sig_handler();
    alarm(timeout_);
    //pthread_mutex_lock(&stat_m_);
    stat_m_.lock();
    std::unordered_map<const char *, ValueTimesPair>::iterator it;
    for (it = statistics_.begin(); it != statistics_.end(); it++) {
        res->statistics[std::string(it->first)] = it->second;
    }

    for (auto& pair : stats_) {
        auto& name = pair.first; 
        auto& stat = pair.second;
        auto ss = stat->reset();
        verify(ss.sum_ >= 0);
        verify(ss.n_stat_ >= 0);
        Log_info("stat name: %s, value: %lld, times: %lld", 
                name.c_str(), ss.sum_, ss.n_stat_);   
        res->statistics[name].value = ss.sum_;
        res->statistics[name].times = ss.n_stat_;
    }

    //pthread_mutex_unlock(&stat_m_);
    stat_m_.unlock();
}

ServerControlServiceImpl::ServerControlServiceImpl(unsigned int timeout,
        Recorder *recorder) :
    recorder_(recorder),
    status_(SCS_INIT),
    timeout_(timeout),
    sig_handler_set_(false) {

    //pthread_mutex_init(&stat_m_, NULL);
    //pthread_mutex_init(&status_mutex_, NULL);
    //pthread_cond_init(&status_cond_, NULL);
    scsi_s = this;
}

ServerControlServiceImpl::~ServerControlServiceImpl() {
    //pthread_mutex_destroy(&stat_m_);
    //pthread_mutex_destroy(&status_mutex_);
    //pthread_cond_destroy(&status_cond_);
}

void ServerControlServiceImpl::set_ready() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    status_ = SCS_RUN;
    rrr::CPUInfo::cpu_stat();
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ServerControlServiceImpl::wait_for_shutdown() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    while (SCS_STOP != status_)
        //pthread_cond_wait(&status_cond_, &status_mutex_);
        status_cond_.wait(status_mutex_);
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

bool operator<(const struct timespec &lhs, const struct timespec &rhs) {
    if (lhs.tv_sec < rhs.tv_sec)
        return true;
    else if (lhs.tv_sec == rhs.tv_sec)
        if (lhs.tv_nsec < rhs.tv_nsec)
            return true;
        else
            return false;
    else
        return false;
}

void clock_gettime(struct timespec *time) {
#ifdef __APPLE__ // OS X
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    time->tv_sec = mts.tv_sec;
    time->tv_nsec = mts.tv_nsec;
#else
    ::clock_gettime(CLOCK_REALTIME, time);
#endif
}

double timespec2ms(struct timespec time) {
    return time.tv_sec * 1000.0 + time.tv_nsec / 1000000.0;
}

void ClientControlServiceImpl::client_shutdown() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    status_ = CCS_STOP;
    //pthread_cond_broadcast(&status_cond_);
    status_cond_.bcast();
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ClientControlServiceImpl::client_force_stop() {
    int i = 0;
    for (; i < num_threads_; i++)
        if (coo_threads_[i] != NULL)
            pthread_kill(*(coo_threads_[i]), SIGALRM);
}

void ClientControlServiceImpl::client_response(ClientResponse* res) {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    if (CCS_FINISH == status_)
        res->is_finish = (rrr::i32)1;
    else
        res->is_finish = (rrr::i32)0;
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();

    pthread_rwlock_wrlock(&collect_lock_);
    before_last_time_ = last_time_;
    clock_gettime(&last_time_);
    res->run_sec = (rrr::i64)(last_time_.tv_sec - start_time_.tv_sec);
    res->run_nsec = (rrr::i64)(last_time_.tv_nsec - start_time_.tv_nsec);

    res->period_sec = (rrr::i64)(last_time_.tv_sec - before_last_time_.tv_sec);
    res->period_nsec = (rrr::i64)(last_time_.tv_nsec - before_last_time_.tv_nsec);

    txn_info_switch_ = !txn_info_switch_;

    for (int i = 0; i < num_threads_; i++) {
        for (std::map<int32_t, txn_info_t>::iterator it = txn_info_[i].begin();
                it != txn_info_[i].end(); it++) {
            res->txn_info[it->first].start_txn += it->second.start_txn;
            res->txn_info[it->first].total_txn += it->second.total_txn;
            res->txn_info[it->first].total_try += it->second.total_try;
            res->txn_info[it->first].commit_txn += it->second.commit_txn;
            res->txn_info[it->first].interval_latency.insert(
                    res->txn_info[it->first].interval_latency.end(),
                    it->second.interval_latency.begin(),
                    it->second.interval_latency.end());
            it->second.interval_latency.clear();
        }
    }
    pthread_rwlock_unlock(&collect_lock_);

    int use = 0;
    if (txn_info_switch_)
        use = 1;
    for (int i = 0; i < num_threads_; i++) {
        for (std::map<int32_t, txn_info_t>::iterator it = txn_info_[i].begin();
                it != txn_info_[i].end(); it++) {

            res->txn_info[it->first].this_latency.insert(
                    res->txn_info[it->first].this_latency.end(),
                    it->second.interval_latencies[use]->begin(), it->second.interval_latencies[use]->end());
            res->txn_info[it->first].last_latency.insert(
                    res->txn_info[it->first].last_latency.end(),
                    it->second.last_interval_latencies[use]->begin(), it->second.last_interval_latencies[use]->end());
            res->txn_info[it->first].attempt_latency.insert(
                    res->txn_info[it->first].attempt_latency.end(),
                    it->second.interval_attempt_latencies[use]->begin(), it->second.interval_attempt_latencies[use]->end());

            res->txn_info[it->first].num_try.insert(
                    res->txn_info[it->first].num_try.end(),
                    it->second.num_try[use]->begin(), it->second.num_try[use]->end());

            it->second.num_try[use]->clear();
            it->second.interval_latencies[use]->clear();
            it->second.last_interval_latencies[use]->clear();
            it->second.interval_attempt_latencies[use]->clear();
        }
    }
}

void ClientControlServiceImpl::client_ready_block(rrr::i32 *res,
        rrr::DeferredReply *defer) {
    *res = 1;
    bool reply = false;
    status_mutex_.lock();
    if (CCS_READY == status_)
        reply = true;
    else
        ready_block_defers_.push_back(defer);
    status_mutex_.unlock();
    if (reply)
        defer->reply();
}

void ClientControlServiceImpl::client_ready(rrr::i32* res) {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    if (CCS_READY == status_)
        *res = 1;
    else
        *res = 0;
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ClientControlServiceImpl::client_start() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    status_ = CCS_RUN;
    //pthread_cond_broadcast(&status_cond_);
    status_cond_.bcast();
    clock_gettime(&start_time_);
    last_time_ = start_time_;
    before_last_time_ = start_time_;
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

//void ClientControlServiceImpl::set_ready() {
//    pthread_mutex_lock(&status_mutex_);
//    status_ = CCS_READY;
//    pthread_mutex_unlock(&status_mutex_);
//}

void ClientControlServiceImpl::wait_for_start(unsigned int id) {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    coo_threads_[id] = (pthread_t *)malloc(sizeof(pthread_t));
    *(coo_threads_[id]) = pthread_self();
    if (++num_ready_ == num_threads_) {
        status_ = CCS_READY;
        for (auto it : ready_block_defers_) {
            it->reply();
        }
        ready_block_defers_.clear();
    }
    while (CCS_RUN != status_ && CCS_STOP != status_)
        //pthread_cond_wait(&status_cond_, &status_mutex_);
        status_cond_.wait(status_mutex_);
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ClientControlServiceImpl::wait_for_shutdown() {
    //pthread_mutex_lock(&status_mutex_);
    status_mutex_.lock();
    if (status_ != CCS_STOP) {
        if (++num_finish_ == num_threads_)
            status_ = CCS_FINISH;
        while (CCS_STOP != status_)
            //pthread_cond_wait(&status_cond_, &status_mutex_);
            status_cond_.wait(status_mutex_);
    }
    //pthread_mutex_unlock(&status_mutex_);
    status_mutex_.unlock();
}

void ClientControlServiceImpl::client_get_txn_names(std::map<i32, std::string> *txn_names) {
    *txn_names = txn_names_;
}

ClientControlServiceImpl::ClientControlServiceImpl(unsigned int num_threads, const std::map<int32_t, std::string> &txn_types) : status_(CCS_INIT), txn_info_(NULL), num_threads_(num_threads), num_ready_(0), num_finish_(0) {
    pthread_rwlock_init(&collect_lock_, NULL);
    //pthread_mutex_init(&status_mutex_, NULL);
    //pthread_cond_init(&status_cond_, NULL);
    coo_threads_ = (pthread_t **)malloc(sizeof(pthread_t *) * num_threads_);
    txn_info_ = new std::map<int32_t, txn_info_t>[num_threads_];
    txn_info_switch_ = true;
    for (int i = 0; i < num_threads_; i++)
        for (std::map<int32_t, std::string>::const_iterator cit = txn_types.begin();
                cit != txn_types.end(); cit++) {
            txn_info_[i][cit->first].init(cit->first);
        }
    txn_names_ = txn_types;
}

ClientControlServiceImpl::~ClientControlServiceImpl() {
    pthread_rwlock_destroy(&collect_lock_);
    //pthread_mutex_destroy(&status_mutex_);
    //pthread_cond_destroy(&status_cond_);
    int i = 0;
    for (; i < num_threads_; i++) {
        for (std::map<int32_t, txn_info_t>::iterator it = txn_info_[i].begin();
                it != txn_info_[i].end(); it++)
            it->second.destroy();
        if (coo_threads_[i] != NULL)
            free(coo_threads_[i]);
    }
    free(coo_threads_);
    delete [] txn_info_;
}

}
 
#ifndef __HMC_H
#define __HMC_H

#include <map>
#include "memory_system.h"


namespace dramcore {

enum class HMCReqType {
    RD16,
    RD32,
    RD48,
    RD64,
    RD80,
    RD96,
    RD112,
    RD128,
    RD256,
    WR16,
    WR32,
    WR48,
    WR64,
    WR80,
    WR96,
    WR112,
    WR128,
    WR256,
    P_WR16,
    P_WR32,
    P_WR48,
    P_WR64,
    P_WR80,
    P_WR96,
    P_WR112,
    P_WR128,
    P_WR256,
    ADD8,  // 2ADD8, cannot name it like that...
    ADD16,
    P_2ADD8,
    P_ADD16,
    ADDS8R,  // 2ADD8, cannot name it like that...
    ADDS16R,
    INC8,
    P_INC8,
    XOR16,
    OR16,
    NOR16,
    AND16,
    NAND16,
    CASGT8,
    CASGT16,
    CASLT8,
    CASLT16,
    CASEQ8,
    CASZERO16,
    EQ8,
    EQ16,
    BWR,
    P_BWR,
    BWR8R,
    SWAP16,
    SIZE
};

enum class HMCRespType {
    NONE,
    RD_RS,
    WR_RS,
    ERR,
    SIZE
};

// for future use
enum class HMCLinkType {
    HOST_TO_DEV,
    DEV_TO_DEV,
    SIZE
};


class HMCRequest {
public:
    HMCRequest(HMCReqType req_type, uint64_t hex_addr);
    HMCReqType type;
    uint64_t mem_operand;
    int link;
    int quad;
    int vault;
    int flits;
    // this exit_time is the time to exit xbar to vaults
    uint64_t exit_time;
};


class HMCResponse {
public:
    HMCResponse(uint64_t id, HMCReqType reqtype, int dest_link, int src_quad);
    uint64_t resp_id;
    HMCRespType type;
    int link;
    int quad;
    int flits;
    // this exit_time is the time to exit xbar to cpu
    uint64_t exit_time;
};


class HMCMemorySystem : public BaseMemorySystem {
public:
    HMCMemorySystem(const std::string &config_file, std::function<void(uint64_t)> callback);
    ~HMCMemorySystem();
    // assuming there are 2 clock domains one for logic die one for DRAM
    // we can unify them as one but then we'll have to convert all the 
    // slow dram time units to faster logic units...
    void ClockTick() override ;
    void LogicClockTickPre();
    void LogicClockTickPost();
    void DRAMClockTick();
    // had to have 3 insert interfaces cuz HMC is so different...
    bool InsertReq(uint64_t hex_addr, bool is_write) override ;
    bool InsertReqToLink(HMCRequest* req, int link);
    bool InsertHMCReq(HMCRequest* req);

    //ThermalCalculator* ptr_thermCal_;

private:
    uint64_t ref_tick_, logic_clk_;
    /*
    uint64_t logic_counter_, dram_counter_;
    int logic_time_inc_, dram_time_inc_;
    uint64_t time_lcm_;
     */
    std::vector<Controller*> vaults_;
    std::function<void(uint64_t)> vault_callback_;

    void SetClockRatio();
    void InsertReqToDRAM(HMCRequest *req); 
    void VaultCallback(uint64_t req_id);
    std::vector<int> BuildAgeQueue(std::vector<int>& age_counter);
    void XbarArbitrate();
    inline void IterateNextLink();

    int next_link_;
    int links_;
    int queue_depth_;
    int dram_clk_ticks_;
    int logic_clk_ticks_;
    int clk_tick_product_;

    // had to use a multimap because the controller callback return hex addr instead of unique id
    std::multimap<uint64_t, HMCResponse*> resp_lookup_table;
    // these are essentially input/output buffers for xbars
    std::vector<std::vector<HMCRequest*>> link_req_queues_;
    std::vector<std::vector<HMCResponse*>> link_resp_queues_;
    std::vector<std::vector<HMCRequest*>> quad_req_queues_;
    std::vector<std::vector<HMCResponse*>> quad_resp_queues_;

    // input/output busy indicators, since each packet could be several
    // flits, as long as this != 0 then they're busy 
    std::vector<int> link_busy;
    std::vector<int> quad_busy = {0, 0, 0, 0};
    // used for arbitration
    std::vector<int> link_age_counter;
    std::vector<int> quad_age_counter = {0, 0, 0, 0};

};


}

#endif


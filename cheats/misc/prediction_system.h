#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

enum Prediction_stage
{
    SETUP,
    PREDICT,
    FINISH
};

struct m_player_data
{
    m_player_data()
    {
        reset();
    };
    ~m_player_data()
    {
        reset();
    };

    void reset()
    {
        m_aimPunchAngle.Zero();
        m_aimPunchAngleVel.Zero();
        m_viewPunchAngle.Zero();

        m_vecViewOffset.Zero();
        m_vecBaseVelocity.Zero();
        m_vecVelocity.Zero();
        m_vecOrigin.Zero();

        m_flFallVelocity = 0.0f;
        m_flVelocityModifier = 0.0f;
        m_flDuckAmount = 0.0f;
        m_flDuckSpeed = 0.0f;
        m_fAccuracyPenalty = 0.0f;
        m_flThirdpersonRecoil = 0.0f;

        m_hGroundEntity = 0;
        m_nMoveType = 0;
        m_nFlags = 0;
        m_nTickBase = 0;
        m_flRecoilIndex = 0;
        tick_count = 0;
        command_number = INT_MAX;
        is_filled = false;
    }

    Vector m_aimPunchAngle = {};
    Vector m_aimPunchAngleVel = {};
    Vector m_viewPunchAngle = {};

    Vector m_vecViewOffset = {};
    Vector m_vecBaseVelocity = {};
    Vector m_vecVelocity = {};
    Vector m_vecOrigin = {};

    float m_flFallVelocity = 0.0f;
    float m_flVelocityModifier = 0.0f;
    float m_flThirdpersonRecoil = 0.0f;
    float m_flDuckAmount = 0.0f;
    float m_flDuckSpeed = 0.0f;
    float m_fAccuracyPenalty = 0.0f;

    int m_hGroundEntity = 0;
    int m_nMoveType = 0;
    int m_nFlags = 0;
    int m_nTickBase = 0;
    int m_flRecoilIndex = 0;

    int tick_count = 0;
    int command_number = INT_MAX;

    bool is_filled = false;
};

struct c_tickbase_array
{
    int command_num = -1;
    int tickbase_original = 0;
    int tickbase_this_tick = 0;
    int extra_cmds = 0;
    int extra_shift = 0;
    bool m_set = false;
    bool increace = false;
    bool doubletap = false;
    bool charge = false;
};

class StoredData_t
{
public:
    int    m_tickbase;
    Vector  m_punch;
    Vector  m_punch_vel;
    Vector m_view_offset;
    float  m_velocity_modifier;

public:
    __forceinline StoredData_t() : m_tickbase{ }, m_punch{ }, m_punch_vel{ }, m_view_offset{ }, m_velocity_modifier{ } {};
};

class engineprediction : public singleton <engineprediction>
{
    struct Netvars_data
    {
        Netvars_data() {
            reset();
        };
        ~Netvars_data() {
            reset();
        };
        float m_duckAmount = 0.f;
        float m_duckSpeed = 0.f;
        int tickbase = INT_MIN;
        void reset()
        {
            command_number = INT_MAX;

            m_aimPunchAngle.Zero();
            m_aimPunchAngleVel.Zero();
            m_viewPunchAngle.Zero();
            m_vecViewOffset.Zero();
            m_vecVelocity.Zero();
            m_vecOrigin.Zero();


            m_flDuckAmount = FLT_MIN;
            m_flThirdpersonRecoil = FLT_MIN;
            m_flFallVelocity = FLT_MIN;
            m_flVelocityModifier = FLT_MIN;

            m_bIsFilled = false;
        }

        int command_number = INT_MAX;

        Vector m_aimPunchAngle = ZERO;
        Vector m_aimPunchAngleVel = ZERO;
        Vector m_viewPunchAngle = ZERO;
        Vector m_vecViewOffset = ZERO;
        Vector m_vecVelocity = ZERO;
        Vector m_vecOrigin = ZERO;

        float m_flDuckAmount = FLT_MIN;
        float m_flThirdpersonRecoil = FLT_MIN;
        float m_flFallVelocity = FLT_MIN;
        float m_flVelocityModifier = FLT_MIN;

        bool m_bIsFilled = false;
    };

    struct Backup_data
    {
        int flags = 0;
        Vector velocity = ZERO;
        float sv_footsteps_backup = 0.0f;
        float sv_min_jump_landing_sound_backup = 0.0f;
    };

    class m_nsequence {
    public:
        float m_time;
        int   m_state;
        int   m_seq;

    public:
        __forceinline m_nsequence() : m_time{ }, m_state{ }, m_seq{ } {};
        __forceinline m_nsequence(float time, int state, int seq) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
    };

    std::array< StoredData_t, MULTIPLAYER_BACKUP > m_data;

    struct Prediction_data
    {
        void reset()
        {
            prediction_stage = SETUP;
            old_curtime = 0.0f;
            old_frametime = 0.0f;
        }

        Prediction_stage prediction_stage = SETUP;
        float old_curtime = 0.0f;
        float old_frametime = 0.0f;
        int* prediction_random_seed = nullptr;
        int* prediction_player = nullptr;
        ConVar* sv_footsteps = nullptr;
        ConVar* sv_min_jump_landing_sound = nullptr;
        int m_nServerCommandsAcknowledged;
        bool m_bInPrediction;
    };
    float m_spread, m_inaccuracy;
    struct Viewmodel_data
    {
        weapon_t* weapon = nullptr;

        int viewmodel_index = 0;
        int sequence = 0;
        int animation_parity = 0;

        float cycle = 0.0f;
        float animation_time = 0.0f;
    };
public:
    Netvars_data netvars_data[MULTIPLAYER_BACKUP];
    Backup_data backup_data;
    Prediction_data prediction_data;
    Viewmodel_data viewmodel_data;
    float get_spread() const { return m_spread; }
    float get_inaccuracy() const { return m_inaccuracy; }
    std::deque< m_nsequence > m_sequence;

    m_player_data m_Data[150] = { };
    c_tickbase_array m_tickbase_array[150] = {};

    void FixNetvarCompression(int time);

    void store_netvars();
    void store_data();
    void reset_data();

    void update_vel();
    void update_incoming_sequences();

    void setup();
    void predict(CUserCmd* m_pcmd);
    void OnFrameStageNotify(ClientFrameStage_t Stage);
    void finish();

};
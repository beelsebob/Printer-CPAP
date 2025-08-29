#pragma once

#include "ESC/BLHeli/BLHeliESCConfig.hpp"
#include "ESC/BLHeli/BootloaderCommand.hpp"
#include "ESC/ESC.hpp"
#include "Utilities/Void.hpp"
#include "Utilities/to_stringExtras.hpp"

#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "driver/uart.h"

#include <string.h>
#include <format>
#include <mutex>
#include <optional>
#include <semaphore>
#include <string>
#include <vector>

namespace pcp {
    enum ProgramModeEntryStep {
        ReadyForRebootSequence,
        RebootingESC,
        ReadyForUART,
        BeginningUART,
    };

    class BLHeliControlSchemeUART {
    public:
        using Completion = std::function<void(bool)>;

        inline uint8_t to_uint8(BootloaderCommandType cmd) { return static_cast<uint8_t>(cmd); }

        BLHeliControlSchemeUART();
        ~BLHeliControlSchemeUART();

        void connect(Completion completion = [](bool x) {});

        ESCState escState(void) const { return _escState; }

        std::optional<BLHeliESCConfig> escConfig(void) const {
            return _escState == ESCState::Programming ? std::optional<BLHeliESCConfig>(_esc) : std::optional<BLHeliESCConfig>();
        }

        BootloaderResult<std::vector<uint8_t>> readMemory(uint16_t address, uint8_t length, TickType_t timeout = 200 / portTICK_PERIOD_MS);

    private:
        void _task(void);

        void _attemptConnection(void);
        bool _setupTimers(void);
        void _cleanupTimer(void);
        bool _configureGeneratorForPulse(void);
        void _transmitPreamble(void);
        void _preambleDidEnd(void);
        void _beginUART(void);
        void _retryConnection(void);
        void _connectionFinished(bool success);

        BootloaderResult<BLHeliESCConfig> _getDeviceConfig(const uint8_t* handshake);

        size_t _writeBytes(const uint8_t* bytes, size_t length, bool crc);
        size_t _readBytes(uint8_t* bytes, size_t length, TickType_t timeout);

        template <typename T>
        size_t expectedReturnBytes();

        template <typename T>
        T getReadBytes(const uint8_t* buffer, size_t length);

        template <BootloaderCommandType cmd>
        BootloaderResult<typename BootloaderCommand<cmd>::ReturnType> _runCommand(BootloaderCommand<cmd> command, TickType_t timeout);

        BootloaderResult<Void> _setAddress(uint16_t address, TickType_t timeout);
        // BootloaderResult<Void> _setBuffer(uint16_t length, TickType_t timeout);
        BootloaderResult<std::vector<uint8_t>> _readMemory(uint8_t length, TickType_t timeout);

        mcpwm_timer_handle_t _timerHandle;
        mcpwm_oper_handle_t _operatorHandle;
        mcpwm_gen_handle_t _generatorHandle;
        mcpwm_cmpr_handle_t _comparatorHandle;

        ESCState _escState;
        ProgramModeEntryStep _programModeEntryStep;

        uint8_t _preamblePulseNumber;
        bool _timerStopping = false;

        QueueHandle_t _uartQueue = nullptr;
        SemaphoreHandle_t _taskSemaphore;
        std::vector<Completion> _connectionCompletions;
        TaskHandle_t _uartTask = nullptr;
        size_t _numRetries = 0;

        BLHeliESCConfig _esc;

        friend void _uartTaskF(void*);
        friend bool _timerEmpty(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
        friend bool _timerStopped(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
        friend class ESCDevice;
    };
}  // namespace pcp

////////////////////////////////////////////////////////////////////////////////////////////////////
// fileName: ScreenSystemView.cpp
//
// Description: TouchGFX System Health Monitor View Implementation
//             Real-time display of system metrics via text areas (C API Version)
//
// Author: Generated for Enovation Controls TouchGFX Project
//
// Copyright: Enovation Controls 2024
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gui/screensystem_screen/ScreenSystemView.hpp>
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>

ScreenSystemView::ScreenSystemView() : updateCounter(0)
{
}

void ScreenSystemView::setupScreen()
{
    ScreenSystemViewBase::setupScreen();

    // Initialize system health manager and get current metrics
    SystemHealth_Init();
    SystemHealth_UpdateMetrics();
    updateAllMetrics();
}

void ScreenSystemView::tearDownScreen()
{
    ScreenSystemViewBase::tearDownScreen();
}

void ScreenSystemView::handleTickEvent()
{
    ScreenSystemViewBase::handleTickEvent();

    // Update every 60 ticks (approximately 1 second at 60 FPS)
    updateCounter++;
    if (updateCounter >= 60)
    {
        updateCounter = 0;
        SystemHealth_UpdateMetrics();
        updateAllMetrics();
    }
}

void ScreenSystemView::buttonHandler(uint8_t value)
{
    // Handle button presses for your system health screen
    switch(value)
    {
        case 0:
            // Button 0 pressed - refresh metrics immediately
            SystemHealth_UpdateMetrics();
            updateAllMetrics();
            break;
        case 1:
            // Button 1 pressed - could reset counters or navigate
            break;
        case 2:
            // Button 2 pressed - could show detailed view
            break;
        case 3:
            // Button 3 pressed - could go back to main menu
            break;
        default:
            break;
    }
}

void ScreenSystemView::updateAllMetrics()
{
    updateMemoryMetrics();
    updateSystemMetrics();
    updateStatusTexts();
}

void ScreenSystemView::updateMemoryMetrics()
{
    const MemoryInfo_t* memInfo = SystemHealth_GetMemoryInfo();

    // Update RAM Usage - Check if text areas exist before using them
    #ifdef TEXTAREARAMVALUE_SIZE
    formatMemoryText(textAreaRAMValueBuffer, TEXTAREARAMVALUE_SIZE,
                    memInfo->usedRAM, memInfo->totalRAM, memInfo->ramPercentage, true);
    textAreaRAMValue.invalidate();
    #endif

    // Update Flash Memory
    #ifdef TEXTAREAFLASHVALUE_SIZE
    formatMemoryText(textAreaFlashValueBuffer, TEXTAREAFLASHVALUE_SIZE,
                    memInfo->usedFlash, memInfo->totalFlash, memInfo->flashPercentage, true);
    textAreaFlashValue.invalidate();
    #endif

    // Update Heap Usage
    #ifdef TEXTAREAHEAPVALUE_SIZE
    formatMemoryText(textAreaHeapValueBuffer, TEXTAREAHEAPVALUE_SIZE,
                    memInfo->usedHeap, memInfo->totalHeap, memInfo->heapPercentage, false);
    textAreaHeapValue.invalidate();
    #endif

    // Update Stack Usage
    #ifdef TEXTAREASTACKVALUE_SIZE
    touchgfx::Unicode::snprintf(textAreaStackValueBuffer, TEXTAREASTACKVALUE_SIZE,
                               "HWM: %d words (%d%%)",
                               memInfo->stackHighWaterMark,
                               memInfo->stackPercentage);
    textAreaStackValue.invalidate();
    #endif
}

void ScreenSystemView::updateSystemMetrics()
{
    const SystemInfo_t* sysInfo = SystemHealth_GetSystemInfo();

    // Update CPU Usage
    #ifdef TEXTAREACPUVALUE_SIZE
    touchgfx::Unicode::snprintf(textAreaCPUValueBuffer, TEXTAREACPUVALUE_SIZE, "%d%%", sysInfo->cpuUsage);
    textAreaCPUValue.invalidate();
    #endif

    // Update Temperature
    #ifdef TEXTAREATEMPVALUE_SIZE
    touchgfx::Unicode::snprintf(textAreaTempValueBuffer, TEXTAREATEMPVALUE_SIZE, "%d°C", sysInfo->temperature);
    textAreaTempValue.invalidate();
    #endif

    // Update Power Supply
    #ifdef TEXTAREAPOWERVALUE_SIZE
    touchgfx::Unicode::snprintf(textAreaPowerValueBuffer, TEXTAREAPOWERVALUE_SIZE,
                               "%.1fV / %dmA",
                               sysInfo->powerVoltage / 1000.0f,
                               sysInfo->powerCurrent);
    textAreaPowerValue.invalidate();
    #endif

    // Update System Information
    #ifdef TEXTAREASYSTEMINFO_SIZE
    touchgfx::Unicode::snprintf(textAreaSystemInfoBuffer, TEXTAREASYSTEMINFO_SIZE,
                               "%dMHz PLL | Tasks: %d/%d",
                               sysInfo->systemClockMHz,
                               sysInfo->activeTaskCount,
                               sysInfo->maxTaskCount);
    textAreaSystemInfo.invalidate();
    #endif

    // Update Uptime
    #ifdef TEXTAREAUPTIME_SIZE
    formatUptimeText(textAreaUptimeBuffer, TEXTAREAUPTIME_SIZE, sysInfo->uptimeSeconds);
    textAreaUptime.invalidate();
    #endif
}

void ScreenSystemView::updateStatusTexts()
{
    // Update status text areas with colored status strings
    #ifdef TEXTAREARAMSTATUS_SIZE
    textAreaRAMStatus.setColor(getStatusColor(SystemHealth_GetRAMStatus()));
    touchgfx::Unicode::strncpy(textAreaRAMStatusBuffer, getStatusString(SystemHealth_GetRAMStatus()), TEXTAREARAMSTATUS_SIZE-1);
    textAreaRAMStatus.invalidate();
    #endif

    #ifdef TEXTAREACPUSTATUS_SIZE
    textAreaCPUStatus.setColor(getStatusColor(SystemHealth_GetCPUStatus()));
    touchgfx::Unicode::strncpy(textAreaCPUStatusBuffer, getStatusString(SystemHealth_GetCPUStatus()), TEXTAREACPUSTATUS_SIZE-1);
    textAreaCPUStatus.invalidate();
    #endif

    #ifdef TEXTAREATEMPSTATUS_SIZE
    textAreaTempStatus.setColor(getStatusColor(SystemHealth_GetTemperatureStatus()));
    touchgfx::Unicode::strncpy(textAreaTempStatusBuffer, getStatusString(SystemHealth_GetTemperatureStatus()), TEXTAREATEMPSTATUS_SIZE-1);
    textAreaTempStatus.invalidate();
    #endif

    #ifdef TEXTAREAPOWERSTATUS_SIZE
    textAreaPowerStatus.setColor(getStatusColor(SystemHealth_GetPowerStatus()));
    touchgfx::Unicode::strncpy(textAreaPowerStatusBuffer, getStatusString(SystemHealth_GetPowerStatus()), TEXTAREAPOWERSTATUS_SIZE-1);
    textAreaPowerStatus.invalidate();
    #endif

    #ifdef TEXTAREASYSTEMSTATUS_SIZE
    textAreaSystemStatus.setColor(getStatusColor(SystemHealth_GetSystemStatus()));
    touchgfx::Unicode::strncpy(textAreaSystemStatusBuffer, getStatusString(SystemHealth_GetSystemStatus()), TEXTAREASYSTEMSTATUS_SIZE-1);
    textAreaSystemStatus.invalidate();
    #endif
}

const char* ScreenSystemView::getStatusString(HealthStatus_t status)
{
    switch(status)
    {
        case STATUS_OK:
            return "OK";
        case STATUS_WARNING:
            return "WARNING";
        case STATUS_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

touchgfx::colortype ScreenSystemView::getStatusColor(HealthStatus_t status)
{
    switch(status)
    {
        case STATUS_OK:
            return touchgfx::Color::getColorFromRGB(0, 255, 136);    // Bright Green
        case STATUS_WARNING:
            return touchgfx::Color::getColorFromRGB(255, 170, 0);    // Orange
        case STATUS_CRITICAL:
            return touchgfx::Color::getColorFromRGB(255, 68, 68);    // Red
        default:
            return touchgfx::Color::getColorFromRGB(255, 255, 255);  // White
    }
}

void ScreenSystemView::formatMemoryText(touchgfx::Unicode::UnicodeChar* buffer,
                                       uint16_t bufferSize,
                                       uint32_t used, uint32_t total,
                                       uint8_t percentage, bool inKB)
{
    if (inKB)
    {
        touchgfx::Unicode::snprintf(buffer, bufferSize,
                                   "%d KB / %d KB (%d%%)",
                                   used / 1024,
                                   total / 1024,
                                   percentage);
    }
    else
    {
        touchgfx::Unicode::snprintf(buffer, bufferSize,
                                   "%d B / %d B (%d%%)",
                                   used,
                                   total,
                                   percentage);
    }
}

void ScreenSystemView::formatUptimeText(touchgfx::Unicode::UnicodeChar* buffer,
                                       uint16_t bufferSize,
                                       uint32_t seconds)
{
    uint32_t days = seconds / 86400;
    uint32_t hours = (seconds % 86400) / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;

    if (days > 0)
    {
        touchgfx::Unicode::snprintf(buffer, bufferSize, "%dd %02d:%02d:%02d", days, hours, minutes, secs);
    }
    else
    {
        touchgfx::Unicode::snprintf(buffer, bufferSize, "%02d:%02d:%02d", hours, minutes, secs);
    }
}

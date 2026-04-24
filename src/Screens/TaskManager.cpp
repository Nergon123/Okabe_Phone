#include "TaskManager.h"
#include <System/LanguageSystem.h>
#ifdef IDF_VER
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif
void ListTasks() {
#ifdef INC_FREERTOS_H
    UBaseType_t  taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t tasks[taskCount];
    uint32_t     totalRunTime[taskCount];
    uxTaskGetSystemState(tasks, taskCount, totalRunTime);
    // drawHeader(getTranslation(TextKey::LM_TASKS_RUNNING), LM_SETTINGS);
    std::vector<mOption> taskList;
    for (UBaseType_t i = 0; i < taskCount; i++) {
        mOption option = mOption("");
        option.label   = NString::format("%s (ID:%d) CPU:%d%% PRIO:%d ST:%d", tasks[i].pcTaskName,
                                         tasks[i].xTaskNumber, totalRunTime[i],
                                         tasks[i].uxCurrentPriority, tasks[i].eCurrentState);
        taskList.push_back(option);
    }
    listMenu(taskList, taskList.size(), true, LM_SETTINGS,
             getTranslation(TextKey::LM_TASKS_RUNNING), false, 0);
#endif
    InfoWindow(getTranslation(TextKey::IW_TASK_LIST_NA));
}

// void ??????????ListAllocations() {
//     drawHeader("Memory Allocations", LM_SETTINGS);
//     std::vector<mOption> allocationList;
//     const char*          sizeSuffixes[3] = {"B", "KB", "MB"};
//     for (const auto& alloc : allocations) {
//         const char* suffix = alloc.size < 1024          ? sizeSuffixes[0]
//                              : alloc.size < 1024 * 1024 ? sizeSuffixes[1]
//                                                         : sizeSuffixes[2];
//         mOption     option = mOption("");

//         option.label = NString::format("%d:/s:%d%s/%s", alloc.id, alloc.size, suffix,
//         alloc.name); allocationList.push_back(option);
//     }
//     listMenu(allocationList, allocationList.size(), true, LM_SETTINGS, "Allocations", false, 0);
// }

void TaskManager() {
    // drawHeader("Task Manager", LM_SETTINGS);
    NString options[2] = {getTranslation(TextKey::LM_TASKS_RUNNING),
                          getTranslation(TextKey::LM_TASKS_EN_RAM_MON)};
    int     selection  = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        options[1] = enableRAMMonitor ? getTranslation(TextKey::LM_TASKS_DIS_RAM_MON)
                                      : getTranslation(TextKey::LM_TASKS_EN_RAM_MON);
        selection  = listMenu(options, ArraySize(options), false, LM_SETTINGS,
                              getTranslation(TextKey::LM_TASKS_MANAGER), false, selection);
        switch (selection) {
        case 0: ListTasks(); break;
        case 1:
            enableRAMMonitor = !enableRAMMonitor;
            drawStatusBar(true);
            break;
        default: break;
        }
    }
}
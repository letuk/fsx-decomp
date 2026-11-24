#include <windows.h>

extern "C" void* __fastcall CopyConstructionDefaults_20161267(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_20161285(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_201612a3(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_201612c1(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_201612df(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_201612fd(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_2016131b(void* receiver);
extern "C" void* __fastcall CopyConstructionDefaults_20161339(void* receiver);

// Minimum receiver layout. Each copy routine initializes the enclosing storage.
template <void* (__fastcall *CopyDefaults)(void*)>
class ConstructionRecordPilot {
public:
    ConstructionRecordPilot();

private:
    DWORD initialState;
};

// Original: api.dll 0x2016198a, 0x2016199e, 0x201619b2, 0x201619c6
// Original: api.dll 0x201619da, 0x201619ee, 0x20161a02, 0x20161a16
// Each constructor uses the copy routine specified by its explicit instantiation.
template <void* (__fastcall *CopyDefaults)(void*)>
ConstructionRecordPilot<CopyDefaults>::ConstructionRecordPilot()
{
    CopyDefaults(this);
    initialState = 1;
}

template class ConstructionRecordPilot<CopyConstructionDefaults_20161267>; // api.dll 0x2016198a
template class ConstructionRecordPilot<CopyConstructionDefaults_20161285>; // api.dll 0x2016199e
template class ConstructionRecordPilot<CopyConstructionDefaults_201612a3>; // api.dll 0x201619b2
template class ConstructionRecordPilot<CopyConstructionDefaults_201612c1>; // api.dll 0x201619c6
template class ConstructionRecordPilot<CopyConstructionDefaults_201612df>; // api.dll 0x201619da
template class ConstructionRecordPilot<CopyConstructionDefaults_201612fd>; // api.dll 0x201619ee
template class ConstructionRecordPilot<CopyConstructionDefaults_2016131b>; // api.dll 0x20161a02
template class ConstructionRecordPilot<CopyConstructionDefaults_20161339>; // api.dll 0x20161a16

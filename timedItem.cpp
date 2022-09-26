
//yo , delete me 
// #if defined(ARDUINO)
// unsigned long GetTime()
// {
//     return millis();
// }
// #else

// // This is a perfect example of the kind of nasty old school C++
// // I'm tryng to keep y'all from having to see. I'm so sorry.
// // this is the standard C++ version of millis()
// #include <chrono>

// unsigned int millis()
// {
//     using namespace std::chrono;
//     return static_cast<uint32_t>(duration_cast<milliseconds>(
//                                      system_clock::now().time_since_epoch())
//                                      .count());
// }

// #endif
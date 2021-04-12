/* Copyright (c) 2009, Fredrik Orderud
   License: BSD licence (http://www.opensource.org/licenses/bsd-license.php) */
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <string>

/// Class for resource management of run-time loaded DLL's
class auto_dll {
public:
    auto_dll (const std::string & dllName) {
        UINT prevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS); // Avoid message boxes when missing dll.
        m_hinstLib       = LoadLibraryA(dllName.c_str());
        SetErrorMode(prevErrMode);
        if (!m_hinstLib)
            throw std::runtime_error("Unable to load "+dllName);
    };
    virtual ~auto_dll () {
        if (m_hinstLib)
            FreeLibrary(m_hinstLib);
    };

    template<typename T>
    T * get_address (const std::string & fun_name) {
        T * proc = 0;
        if (m_hinstLib != NULL)
            proc = reinterpret_cast<T*>(GetProcAddress(m_hinstLib, (fun_name.c_str())));

        return proc;
    }

private:
    HINSTANCE    m_hinstLib;
};
#else
// non-Win32
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class auto_dll {
public:
    auto_dll (std::string const& lib_name) : lib_handle(0) {
        lib_handle = dlopen( lib_name.c_str(), RTLD_LOCAL | RTLD_LAZY );
    }
    ~auto_dll () {
        if (lib_handle)
            dlclose(lib_handle);
    }

    template<typename T>
    T * get_address (std::string const& symbol) {
        return reinterpret_cast<T*>(dlsym(lib_handle, symbol.c_str()));
    }

private:
    void * lib_handle;
};
#endif

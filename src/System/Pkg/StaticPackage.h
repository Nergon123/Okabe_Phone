#ifndef _H_STATICPACKAGE
#define _H_STATICPACKAGE

#include "BasePackage.h"
#include <Platform/NString.h>

#define CREATE_SPKG(name, id, main_fn)                                                            \
    extern const char spkgClass_##name##_id[] = id;                                               \
    StaticPackage     spkgClass_##name(#name, spkgClass_##name##_id, main_fn);

#define DECLARE_SPKG(name)                                                                        \
    extern const char    spkgClass_##name##_id[];                                                 \
    extern StaticPackage spkgClass_##name;

#define SPKG(name) spkgClass_##name

#define SPKG_ID(name) spkgClass_##name##_id

typedef int (*SPkgMainFn)();

class StaticPackage : public BasePackage {
  public:
    StaticPackage(const std::string& name, const std::string& id, SPkgMainFn fn) {
        BasePackageInfo i;
        i.name    = name;
        i.id      = id;
        i.version = "builtin";

        m_info = i;

        m_fn = fn;
    };
    virtual ~StaticPackage() = default;

    int exec() override {
        if (m_fn) { return m_fn(); }
        else { return PKG_ERR; }
    };

  protected:
    SPkgMainFn m_fn;
};

#endif

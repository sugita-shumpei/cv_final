#ifndef CVLIB_UTILS_H
#define CVLIB_UTILS_H
#define CVLIB_DECLARE_GET_BY_REFERENCE(CLASSNAME,TYPE,NAME,PROP) const TYPE& get##NAME()const noexcept { return this->PROP;}
#define CVLIB_DECLARE_SET_BY_REFERENCE(CLASSNAME,TYPE,NAME,PROP) void  set##NAME(const TYPE& NAME) noexcept { this->PROP = NAME;}
#define CVLIB_DECLARE_GET_AND_SET_BY_REFERENCE(CLASSNAME,TYPE,NAME,PROP) \
    const TYPE& get##NAME()const noexcept { return this->PROP;} \
    void set##NAME(const TYPE& NAME) noexcept { this->PROP = NAME;}
#define CVLIB_DECLARE_GET_BY_VALUE(CLASSNAME,TYPE,NAME,PROP)     const TYPE get##NAME()const noexcept { return this->PROP;}
#define CVLIB_DECLARE_SET_BY_VALUE(CLASSNAME,TYPE,NAME,PROP)     void  set##NAME(const TYPE NAME) noexcept { this->PROP = NAME;}
#define CVLIB_DECLARE_GET_AND_SET_BY_VALUE(CLASSNAME,TYPE,NAME,PROP) \
    const TYPE get##NAME()const noexcept { return this->PROP;} \
    void  set##NAME(const TYPE NAME) noexcept { this->PROP = NAME;}
#endif
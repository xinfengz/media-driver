/*
* Copyright (c) 2020-2021, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     mhw_def.h
//! \brief    MHW interface common defines
//! \details
//!

#ifndef __MHW_DEF_H__
#define __MHW_DEF_H__

#include <memory>
#include <vector>
#include "mhw_utilities.h"

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define MHW_HWCMDPARSER_ENABLED (_MHW_HWCMDPARSER_SUPPORTED && (_DEBUG || _RELEASE_INTERNAL))
#if MHW_HWCMDPARSER_ENABLED
#include "mhw_hwcmd_parser.h"
#else
#define MHW_HWCMDPARSER_INIT(osInterface)
#define MHW_HWCMDPARSER_DESTROY()
#define MHW_HWCMDPARSER_PARSEFIELDSLAYOUTEN() false
#define MHW_HWCMDPARSER_PARSEFIELDLAYOUT(dw, field)
#define MHW_HWCMDPARSER_FRAMEINFOUPDATE(frameType)
#define MHW_HWCMDPARSER_PARSECMD(cmdName, cmdData, dwLen)
#define MHW_HWCMDPARSER_PARSECMDBUF(cmdBuf, dwLen)
#define MHW_HWCMDPARSER_PARSECMDBUFGFX(cmdBufGfx)
#endif

#define _MHW_CMD_T(CMD)      CMD##_CMD         // MHW command type
#define __MHW_CMD_PAR_M(CMD) m_##CMD##_Params  // member which is a pointer to MHW command parameters

#define __MHW_CMD_PAR_GET_F(CMD)       GetCmdPar_##CMD       // function name to get the pointer to MHW command parameter
#define __MHW_CMD_BYTE_SIZE_GET_F(CMD) GetCmdByteSize_##CMD  // function name to get MHW command size in byte
#define __MHW_CMD_ADD_F(CMD)           AddCmd_##CMD          // function name to add command
#define __MHW_CMD_SET_F(CMD)           SetCmd_##CMD          // function name to set command data

#define __MHW_CMD_PAR_GET_DECL(CMD)       mhw::Pointer<_MHW_CMD_PAR_T(CMD)> __MHW_CMD_PAR_GET_F(CMD)(bool reset)
#define __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) size_t __MHW_CMD_BYTE_SIZE_GET_F(CMD)() const

#define __MHW_CMD_ADD_DECL(CMD) MOS_STATUS __MHW_CMD_ADD_F(CMD)(PMOS_COMMAND_BUFFER cmdBuf,                  \
                                                                PMHW_BATCH_BUFFER   batchBuf      = nullptr, \
                                                                const uint8_t      *extraData     = nullptr, \
                                                                size_t              extraDataSize = 0)

#if MHW_HWCMDPARSER_ENABLED
#define __MHW_CMD_SET_DECL(CMD) MOS_STATUS __MHW_CMD_SET_F(CMD)(void *cmdData, const std::string &cmdName = #CMD)
#else
#define __MHW_CMD_SET_DECL(CMD) MOS_STATUS __MHW_CMD_SET_F(CMD)(void *cmdData)
#endif

#define _MHW_CMD_SET_DECL_OVERRIDE(CMD) __MHW_CMD_SET_DECL(CMD) override

#define __MHW_CMD_PAR_GET_DEF(CMD)              \
    __MHW_CMD_PAR_GET_DECL(CMD) override        \
    {                                           \
        if (reset)                              \
        {                                       \
            *(this->__MHW_CMD_PAR_M(CMD)) = {}; \
        }                                       \
        return this->__MHW_CMD_PAR_M(CMD);      \
    }

#define __MHW_CMD_BYTE_SIZE_GET_DEF(CMD)                \
    __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) override          \
    {                                                   \
        return sizeof(typename cmd_t::_MHW_CMD_T(CMD)); \
    }

#define __MHW_CMD_ADD_DEF(CMD)                                                                        \
    __MHW_CMD_ADD_DECL(CMD) override                                                                  \
    {                                                                                                 \
        MHW_FUNCTION_ENTER;                                                                           \
        this->m_currentCmdBuf   = cmdBuf;                                                             \
        this->m_currentBatchBuf = batchBuf;                                                           \
        typename cmd_t::_MHW_CMD_T(CMD) cmdData;                                                      \
        MHW_CHK_STATUS_RETURN(this->__MHW_CMD_SET_F(CMD)(&cmdData));                                  \
        MHW_HWCMDPARSER_PARSECMD(#CMD,                                                                \
            reinterpret_cast<uint32_t*>(&cmdData),                                                    \
            sizeof(cmdData) / sizeof(uint32_t));                                                      \
        MHW_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(cmdBuf, batchBuf, &cmdData, sizeof(cmdData)));    \
        if (extraData && extraDataSize > 0)                                                           \
        {                                                                                             \
            MHW_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(cmdBuf, batchBuf, extraData, extraDataSize)); \
        }                                                                                             \
        return MOS_STATUS_SUCCESS;                                                                    \
    }

#define _MHW_CMD_ALL_DEF_FOR_ITF(CMD)              \
public:                                            \
    virtual __MHW_CMD_PAR_GET_DECL(CMD)       = 0; \
    virtual __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) = 0; \
    virtual __MHW_CMD_ADD_DECL(CMD)           = 0

#define _MHW_CMD_ALL_DEF_FOR_IMPL(CMD)                             \
public:                                                            \
    __MHW_CMD_PAR_GET_DEF(CMD);                                    \
protected:                                                         \
    virtual __MHW_CMD_SET_DECL(CMD) { return MOS_STATUS_SUCCESS; } \
    mhw::Pointer<_MHW_CMD_PAR_T(CMD)> __MHW_CMD_PAR_M(CMD) = mhw::MakePointer<_MHW_CMD_PAR_T(CMD)>()

#define _MHW_CMD_ALL_DEF_FOR_IMPL_GENERIC(CMD) \
public:                                        \
    __MHW_CMD_BYTE_SIZE_GET_DEF(CMD);          \
    __MHW_CMD_ADD_DEF(CMD)

#define _MHW_CMDSET_GETCMDPARAMS_AND_CALLBASE(CMD)                                     \
    MHW_FUNCTION_ENTER;                                                                \
    auto        cmd    = reinterpret_cast<typename cmd_t::_MHW_CMD_T(CMD) *>(cmdData); \
    const auto &params = this->__MHW_CMD_PAR_M(CMD);                                   \
    base_t::__MHW_CMD_SET_F(CMD)(cmd)

#define _MHW_SETPARAMS_AND_ADDCMD(CMD, cmdPar_t, GetPar, AddCmd, ...)           \
    {                                                                           \
        auto par = GetPar(CMD, true);                                           \
        auto p   = dynamic_cast<const cmdPar_t *>(this);                        \
        if (p)                                                                  \
        {                                                                       \
            p->__MHW_CMD_PAR_SET_F(CMD)(par);                                   \
        }                                                                       \
        LOOP_FEATURE_INTERFACE_RETURN(cmdPar_t, __MHW_CMD_PAR_SET_F(CMD), par); \
        AddCmd(CMD, __VA_ARGS__);                                               \
    }

// DWORD location of a command field
#define _MHW_CMD_DW_LOCATION(field) \
    static_cast<uint32_t>((reinterpret_cast<uint32_t *>(&(cmd->field)) - reinterpret_cast<uint32_t *>(&(*cmd))))

#define _MHW_CMD_ASSIGN_FIELD(dw, field, value) cmd->dw.field = (value)

namespace mhw
{
inline int32_t Clip3(int32_t x, int32_t y, int32_t z)
{
    int32_t ret = 0;

    if (z < x)
    {
        ret = x;
    }
    else if (z > y)
    {
        ret = y;
    }
    else
    {
        ret = z;
    }

    return ret;
}

inline bool MmcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC || state == MOS_MEMCOMP_MC;
}

inline bool MmcRcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC;
}

inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}
}  // namespace mhw

#endif  // __MHW_DEF_H__
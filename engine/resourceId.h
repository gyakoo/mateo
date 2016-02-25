#pragma once

namespace engine
{
    template<int N>
    struct resourceId
    {
        enum { TYPEMASK = 0xff000000, TYPESHIFT = 24, MAXNUMB = ~TYPEMASK }; // 8 bits type, 24 bits number
        resourceId() { invalidate(); }
        explicit resourceId(uint32_t _number) { invalidate(); number(_number); }

        static inline resourceId<N> createFrom(uint32_t n) { resourceId<N> t; t.id = n; return t; }
        inline uint8_t   type()const { return  (id&TYPEMASK) >> TYPESHIFT; }
        inline uint32_t  number()const { return (id&(~TYPEMASK)); }
        inline void    number(uint32_t n) { id = (id&TYPEMASK) | (n&(~TYPEMASK)); }
        inline bool    isValid()const 
        { 
            return (id&(~TYPEMASK)) != (~TYPEMASK); 
        }
        inline void    invalidate() { id = (N << TYPESHIFT) | (~TYPEMASK); }
        inline bool    operator ==(const resourceId<N>& o) { return id == o.id; }
        inline operator uint32_t() { return id; }
        static resourceId<N> INVALID() { return resourceId<N>(); } // an invalid id has the 24 lsb to 1 
        static uint32_t extractType(uint32_t resId) { return (resId&TYPEMASK) >> TYPESHIFT; }
        static uint32_t extractNumber(uint32_t resId) { return (resId&(~TYPEMASK)); }
    private:
        uint32_t  id;
    };


    enum resourceTypes
    {
        // BASE
        ID_GENERIC = 0,
        ID_FILE,
        ID_JSON,
        ID_LOGWRITER,

        // RENDERER
        ID_RENDERTARGET,
        ID_VERTEXLAYOUT,
        ID_MESHBUFFER,
        ID_TEXTURE,
        ID_BYTECODE,
        ID_SHADER,
        ID_SAMPLERSTATE,
        ID_DEPTHSTENCILSTATE,
        ID_RASTERIZERSTATE,
        ID_BLENDSTATE,

        // INPUT
        ID_INPUTTRIGGER,

        // JOB MANAGER
        ID_JOB,

        // SOUND MANAGER
        ID_SOUND,

        // DYNAMIC LIBRARIES
        ID_DYNLIB,

        // NETWORK
        ID_SOCKET,

        // PHYSICS

        ID_BASERESOURCEID_MAX
    };

    typedef resourceId<ID_GENERIC>            idGeneric;
    typedef resourceId<ID_FILE>               idFile;
    typedef resourceId<ID_JSON>               idJson;
    typedef resourceId<ID_LOGWRITER>          idLogWriter;
    typedef resourceId<ID_RENDERTARGET>       idRenderTarget;
    typedef resourceId<ID_VERTEXLAYOUT>       idVertexLayout;
    typedef resourceId<ID_MESHBUFFER>         idMeshBuffer;
    typedef resourceId<ID_TEXTURE>            idTexture;
    typedef resourceId<ID_BYTECODE>           idByteCode;
    typedef resourceId<ID_SHADER>             idShader;
    typedef resourceId<ID_SAMPLERSTATE>       idSamplerState;
    typedef resourceId<ID_DEPTHSTENCILSTATE>  idDepthStencilState;
    typedef resourceId<ID_RASTERIZERSTATE>    idRasterizerState;
    typedef resourceId<ID_BLENDSTATE>         idBlendState;
    typedef resourceId<ID_INPUTTRIGGER>       idInputTrigger;
    typedef resourceId<ID_JOB>                idJob;
    typedef resourceId<ID_SOUND>              idSound;
    typedef resourceId<ID_DYNLIB>             idDynLib;
    typedef resourceId<ID_SOCKET>             idSocket;

};
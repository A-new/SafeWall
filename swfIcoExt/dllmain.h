// dllmain.h : ģ�����������

class CswfIcoExtModule : public ATL::CAtlDllModuleT< CswfIcoExtModule >
{
public :
	DECLARE_LIBID(LIBID_swfIcoExtLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SWFICOEXT, "{A7AD6289-A7A0-4931-BA15-A3DFB342227D}")
};

extern class CswfIcoExtModule _AtlModule;

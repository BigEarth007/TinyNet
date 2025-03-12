#ifndef __TINYNET_H__
#define __TINYNET_H__
#include <map>
#include <string>
#include <thread>
#include <functional>
#include "Debug.h"

struct sockaddr;
struct sockaddr_in;
#define stSockaddrIn struct sockaddr_in
#define stSockaddr struct sockaddr
#define stIpMreq struct ip_mreq

namespace tinynet
{
	enum class ENetType
	{
		TCP = 0,
		UDP,
		// �鲥������
		UDP_MULTICAST,
	};

	enum class ENetEvent
	{
		// ��������
		Ready = 0,
		// TCP ���յ��ͻ���
		Accept,
		// ������
		Heart,
		// �˳�
		Quit,
	};

	// ��ȡ����CPU����
	inline const unsigned int GetCpuNum();
	// ��ȡ����IP
	const std::string GetLocalIPAddress();

	int LastError();

	inline void CloseSocket(size_t& n_nSocket);

#if defined(_WIN32) || defined(_WIN64)
#define SockaddrLen int
#define ValType const char*
#else
#define SockaddrLen socklen_t
#define ValType const void*
#define SOCKET_ERROR -1
#endif

#define IPADDR_SIZE 16
#define SOCKADDR_SIZE 16

	struct FNetNode
	{
		// Э��
		ENetType		eNetType = ENetType::TCP;
		// Socket
		size_t			fd = 0;
		char			Addr[SOCKADDR_SIZE] = {0};

		void Init(const ENetType n_eType,
			const std::string& n_sHost, const unsigned short n_nPort);

		void Init(const ENetType n_eType, const stSockaddrIn* n_Addr);

		// �˿ں�
		const unsigned short Port();

		// IP
		const std::string Ip();

		/// <summary>
		/// ������Ϣ
		/// </summary>
		/// <param name="n_szData">��Ϣ����</param>
		/// <param name="n_nSize">��Ϣ����</param>
		/// <returns></returns>
		int Send(const char* n_szData, const int n_nSize) const;
		int Send(const std::string& n_sData) const;

		/// <summary>
		/// ����UDP��Ϣ�����������û�
		/// </summary>
		/// <param name="n_szData">��Ϣ����</param>
		/// <param name="n_nSize">��Ϣ����</param>
		/// <param name="n_sHost">Ŀ��IP</param>
		/// <param name="n_nPort">Ŀ��˿�</param>
		/// <returns></returns>
		int Send(const char* n_szData, const int n_nSize,
			const std::string& n_sHost, const unsigned short n_nPort) const;
		int Send(const std::string& n_sData,
			const std::string& n_sHost, const unsigned short n_nPort) const;

		const bool IsValid() const;

		const std::string ToString();

		bool operator==(const FNetNode& n_NetNode);

		void Clear();
	};

#pragma region �鲥
	class INetImpl
	{
	public:
		INetImpl();
		virtual ~INetImpl();

	protected:
		void Startup();
		void Cleanup();

	protected:
#if defined(_WIN32) || defined(_WIN64)
		static int m_nRef;
#endif
	}
#pragma endregion

#pragma region �鲥
	// �鲥
	class CMulticast : public INetImpl
	{
	public:
		CMulticast();
		~CMulticast();

		/// <summary>
		/// ��ʼ��
		/// </summary>
		/// <param name="n_sHost">�鲥IP</param>
		/// <param name="n_nPort">�鲥�˿ں�</param>
		/*
		��ַ��Χ					����

		224.0.0.0��224.0.0.255		�������ַ��IANAΪ·��Э��Ԥ����IP��ַ��Ҳ��Ϊ�������ַ����
									���ڱ�ʶһ���ض��������豸����·��Э�顢���˲��ҵ�ʹ�ã��������鲥ת����

		224.0.1.0��231.255.255.255	ASM�鲥��ַ��ȫ����Χ����Ч��
		233.0.0.0��238.255.255.255	˵�������У�224.0.1.39��224.0.1.40�Ǳ�����ַ��������ʹ�á�

		232.0.0.0��232.255.255.255	ȱʡ����µ�SSM�鲥��ַ��ȫ����Χ����Ч��

		239.0.0.0��239.255.255.255	���ع������ַ�����ڱ��ع���������Ч���ڲ�ͬ�Ĺ��������ظ�ʹ����ͬ�ı��ع������ַ���ᵼ�³�ͻ��
		*/
		bool Init(const std::string& n_sHost, const unsigned short n_nPort);

		/// <summary>
		/// �����鲥(���Ͷ�)
		/// </summary>
		int Sender();

		/// <summary>
		/// �����鲥(���ն�)
		/// </summary>
		/// <param name="n_sLocalIP">����IP��Ϊ�����Զ���ȡ</param>
		/// <returns></returns>
		int Receiver(const std::string& n_sLocalIP = "");

		/// <summary>
		/// �˳��鲥
		/// </summary>
		/// <returns></returns>
		int Release();

		/// <summary>
		/// �����鲥��Ϣ
		/// </summary>
		/// <param name="n_szData">��Ϣ����</param>
		/// <param name="n_nSize">��Ϣ����</param>
		/// <returns></returns>
		int Send(const char* n_szData, const int n_nSize);
		int Send(const std::string n_sData);

		void SetRecvBuffSize(const int n_nSize);

		// ���ݽ��ջص�
		std::function<void(const std::string& )> fnRecvCallback = nullptr;
	protected:
		void MulticastThread() const;

	protected:
		int			m_nBuffSize = 1024;
		FNetNode	m_NetNode;

		std::string m_sLocalIp;
	};
#pragma endregion

#pragma region base
	class ITinyNet : public INetImpl, public FNetNode
	{
	public:
		ITinyNet();
		virtual ~ITinyNet();

		/// <summary>
		/// ����Socket
		/// </summary>
		/// <returns></returns>
		virtual bool Start();
		/// <summary>
		/// �˳�
		/// </summary>
		virtual void Stop();

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="n_nPeriod">��������(����)</param>
		virtual void EnableHeart(int n_nPeriod);
		
		int KeepAlive(int n_nAlive = 1) const;
		// ���ó�ʱ��������Startǰ����
		int SetTimeout(int n_nMilliSeconds);
		// ���� SO_REUSEADDR 
		int ReuseAddr(int n_nReuse) const;

		void SetRecvBuffSize(const int n_nSize);

		// �¼��ص�
		std::function<void(const ENetEvent, const std::string&)> fnEventCallback = nullptr;
		// ���ݽ��ջص�
		std::function<void(FNetNode*, const std::string&)> fnRecvCallback = nullptr;

	protected:
		int			m_nBuffSize = 1024;
		// Ĭ��3�볬ʱ
		int			m_nTimeout = 3000;
		FNetNode	m_NetNode;

		bool		m_bRun = false;
	};
#pragma endregion

	////////////////////////////////////////////////////////////////////////////////
#pragma region server
	class CTinyServer : public ITinyNet
	{
	public:
		CTinyServer();
		~CTinyServer();

		bool Start() override;
		void Stop() override;

	protected:
#if defined(_WIN32) || defined(_WIN64)
		bool InitSock();
		void Accept();
		bool CreateWorkerThread();
		void WorkerThread();

		// Ͷ�ݽ���
		int PostRecv(void* n_Handle);

		// ������ɼ�
		void* AllocSocketNode(size_t n_fd, const stSockaddrIn* n_Addr);
		// �ͷ���ɼ�
		void FreeSocketNode(void** n_Handle);
		// �ͷ�������ɼ�
		void FreeSocketNodes();
#else
		bool InitSock();
		void WorkerThread();

	public:
		void SetEt(const bool et = true);
	protected:
		int SetNonblock(int n_nFd);
		int AddSocketIntoPoll(int n_nFd);
		int DelSocketFromPoll(int n_nFd);
		void FreeSocketNode(int n_nFd);
		void FreeSocketNodes();
#endif

	protected:
#if defined(_WIN32) || defined(_WIN64)
		void*			m_hIocp = nullptr;
		std::thread*	m_threads = nullptr;
		std::map<FNetNode*, void*> m_mNodes;
#else
		int				m_nEpfd = 0;
		bool			m_bEt = true;
		std::map<int, FNetNode> m_mNodes;
#endif
	};
#pragma endregion

	////////////////////////////////////////////////////////////////////////////////
#pragma region client
	class CTinyClient : public ITinyNet
	{
	public:
		CTinyClient();
		~CTinyClient();

		bool Start() override;
		void Stop() override;

		/// <summary>
		/// ����������
		/// </summary>
		/// <param name="n_nPeriod">��������(����)</param>
		void EnableHeart(int n_nPeriod) override;

	protected:
		bool InitSock();
		void WorkerThread();
		void HeartThread(int n_nPeriod);
		void Join();
		// ����������
		int SendHeart();

	protected:
		std::thread	m_thread;

		int			m_nHeart = -1;
	};
#pragma endregion
}


#endif // !__TINYNET_H__

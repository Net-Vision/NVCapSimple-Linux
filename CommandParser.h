// Command Line Parser
// [15/16/19] H.Yamada / NetVision Corp.

#ifndef _COMMAND_PARSER_H_
#define _COMMAND_PARSER_H_

#define CCP_MAX_CHAR 128

class CCommandParser{
public:
	CCommandParser(){
		m_pArgumentData = NULL;
		m_argumentSize = 0;
	};
	virtual ~CCommandParser(){
		if(m_pArgumentData) delete [] m_pArgumentData;
	};

	struct sParseData{
		char argument[CCP_MAX_CHAR]; // starts from "-"
		char value[CCP_MAX_CHAR]; // optional
		char hasValue; // [0, 1]
	};

	int Parse(char** argv, int argc)
	{
		if(m_pArgumentData) delete [] m_pArgumentData;
		if(argc <= 0) return -1;
		m_pArgumentData = new sParseData[argc];
		m_argumentSize = 0;
		int i;
		int nextIsArg = 0;
		// parse
		for(i=0; i<argc; i++){
			if(argv[i][0] == '-'){ // Argument
				strncpy(m_pArgumentData[m_argumentSize].argument, 
					&argv[i][1], CCP_MAX_CHAR-1);
				m_pArgumentData[m_argumentSize].hasValue = 0;
				m_argumentSize ++;
				nextIsArg = 1;
			}else if(nextIsArg){ // possibly Value
				strncpy(m_pArgumentData[m_argumentSize-1].value, 
					argv[i], CCP_MAX_CHAR-1);
				m_pArgumentData[m_argumentSize-1].hasValue = 1;
				nextIsArg = 0;
			}else{ // error
				return 1+i;
			}
		}

		return 0;
	};
	
	int GetArgumentSize(){
		return m_argumentSize;
	};
	const sParseData* GetArgument(int id){
		if(id<0 || id>=m_argumentSize) return NULL;
		else return &(m_pArgumentData[id]);
	};
	
private:
	int m_argumentSize;
	sParseData* m_pArgumentData;
};

#endif


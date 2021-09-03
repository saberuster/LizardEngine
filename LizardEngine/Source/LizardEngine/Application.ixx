export module LizardEngine : Application;

export class IApplication
{
public:
	IApplication(){};
	virtual ~IApplication(){};

	IApplication(const IApplication &app) = delete;
	IApplication &operator=(const IApplication &app) = delete;

	virtual void Init() = 0;
	virtual void Tick() = 0;
	virtual void Quit() = 0;
};

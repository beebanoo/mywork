
#pragma once








#include <assert.h>



enum SpherePackFlag
{
	
};


#pragma warning(push)
#pragma warning(disable:4100)

class SpherePackCallback
{
public:
	SpherePackCallback() {}
	virtual ~SpherePackCallback() {}

	{};
	
	{};
	
		SpherePack *sphere,
	{};

		SpherePack *sphere,
	{};
	
private:
};

#pragma warning(pop)

class SpherePack : public Sphere
{
public:
	SpherePack(void)
	{		
		mPrevious         = 0;
		mParent           = 0;
		mPrevSibling      = 0;
		IS_SPHERE	= false;
	};
	virtual ~SpherePack()
	{
	}
	
	{
		IS_SPHERE			= isSphere;
		mUserData         = userdata;
		mParent           = 0;
		mNextSibling      = 0;
		mPrevSibling      = 0;
		mFlags            = 0;
		mFifo1            = 0;
		mFifo2            = 0;
		mFactory          = factory;
		mCenter           = pos;
		SetRadius(radius);
	};
	
	void SetSpherePackFlag(SpherePackFlag flag) { mFlags|=flag; };
	void ClearSpherePackFlag(SpherePackFlag flag) { mFlags&=~flag; };
	bool HasSpherePackFlag(SpherePackFlag flag) const
	{
		if ( mFlags & flag ) return true;
		return false;
	};
	
	
	void SetParent(SpherePack *pack) { mParent = pack; };
	SpherePack * GetParent(void) const { return mParent; };
	
	inline void NewPos(const Vector3d &pos);
	inline void NewPosRadius(const Vector3d &pos,float radius);
	
	void Unlink(void)
	{
		{
			*mFifo1 = 0;
			mFifo1 = 0;
		}
		
		{
			*mFifo2 = 0;
			mFifo2 = 0;
		}
		
		if ( mParent ) mParent->LostChild(this);
		
		
	}
	
	
	void AddChild(SpherePack *pack)
	{
		
		SpherePack *my_child = mChildren;
		
		pack->SetParent(this);
		
		
		mChildCount++;

#if defined(_DEBUG)
		float dist = DistanceSquared(pack);
		float radius = sqrtf(dist) + pack->GetRadius();
		
		assert( radius <= GetRadius()+0.0001f );
#endif
	}
	
	void SetNextSibling(SpherePack *child) { mNextSibling = child; }
	void SetPrevSibling(SpherePack *child) { mPrevSibling = child; }
	
	SpherePack * _GetNextSibling(void) const 
	{ 
		return mNextSibling; 
	}
	SpherePack * _GetPrevSibling(void) const 
	{ 
		return mPrevSibling; 
	}
	SpherePack * GetChildren(void)    const { return mChildren; }
	
	SpherePack * GetNext(void)     const { return mNext; };
	SpherePack * GetPrevious(void) const { return mPrevious; };
	
	void SetNext(SpherePack *pack) { mNext = pack; };
	void SetPrevious(SpherePack *pack) { mPrevious = pack; };
	
	void * GetUserData(void) const { return mUserData; };
	void   SetUserData(void *data, bool isSphere) { mUserData = data; IS_SPHERE=isSphere;};
	
	float DistanceSquared(const SpherePack *pack) const { return mCenter.DistanceSq( pack->mCenter );  };
	
	void LostChild(SpherePack *pack);
	
	const Vector3d& GetPos(void) const { return mCenter; };
	
	inline void Render(unsigned int color);
	
	inline bool Recompute(float gravy);
	
	int GetChildCount(void) const { return mChildCount; };
	
#if DEMO
	void SetColor(unsigned int color) { mColor = color; };
	unsigned int GetColor(void) const { return mColor; };
#endif
	
	void SetFifo1(SpherePack **fifo)
	{
		mFifo1 = fifo;
	};
	
	void SetFifo2(SpherePack **fifo)
	{
		mFifo2 = fifo;
	};
	
	void ComputeBindingDistance(SpherePack *parent)
	{
		mBindingDistance = parent->GetRadius() - GetRadius();
		if ( mBindingDistance <= 0 )
			mBindingDistance = 0;
		else
			mBindingDistance*=mBindingDistance;
	}
	
	void VisibilityTest(const Frustum &f,
		SpherePackCallback *callback,
		ViewState state);
	
		SpherePackCallback *callback);
	
	
	void RangeTest(const Vector3d &p,
		float distance,
		SpherePackCallback *callback,
		ViewState state);
	
	void PointTest2d(const Vector3d &p,
		SpherePackCallback *callback,
		ViewState state);

	void Reset(void);
	
private:
	SpherePack       *mNext;
	
	SpherePack       *mParent;
	
	
	
	
	float             mBindingDistance;
	
	void             *mUserData;
	
#if DEMO
	unsigned long      mColor;
#endif

public:
	bool				IS_SPHERE;
};

class SpherePackFifo
{
public:
	SpherePackFifo(int fifosize)
	{
		mCount = 0;
		mSP = 0;
		mBottom = 0;
		mFifoSize = fifosize;
		mFifo = new SpherePack *[mFifoSize];
	};
	
	virtual ~SpherePackFifo(void)
	{
		delete [] mFifo;
	};
	
	SpherePack ** Push(SpherePack *sphere)
	{
		mCount++;
		SpherePack **ret = &mFifo[mSP];
		mFifo[mSP] = sphere;
		mSP++;
		if ( mSP == mFifoSize ) mSP = 0;
		return ret;
	};
	
	SpherePack * Pop(void)
	{
		while ( mSP != mBottom )
		{
			mCount--;
			SpherePack *ret = mFifo[mBottom];
			mBottom++;
			if ( mBottom == mFifoSize ) mBottom = 0;
			if ( ret ) return ret;
		}
		return 0;
	}
	
	bool Flush(SpherePack *pack)
	{
		if ( mSP == mBottom ) return false;
		int i = mBottom;
		while ( i != mSP )
		{
			if ( mFifo[i] == pack )
			{
				mFifo[i] = 0;
				return true;
			}
			i++;
			if ( i == mFifoSize ) i = 0;
		}
		return false;
	};
	
	int GetCount(void) const { return mCount; };
	
private:
	int         mCount;
	int         mBottom;
	int         mFifoSize;
	SpherePack **mFifo;
};


class SpherePackFactory : public SpherePackCallback
{
public:
	
	SpherePackFactory(int maxspheres,
		float rootsize,
		float leafsize,
		float gravy);
	
	
	virtual ~SpherePackFactory(void);
	
	void Process(void);
	
	SpherePack *AddSphere_(const Vector3d &pos,
		float radius,
		void *userdata,
		bool isSphere,
		int flags=SPF_LEAF_TREE);
	
	
	void Integrate(SpherePack *pack,SpherePack *supersphere,float node_size);
	
	void Render(void);
	
	void Remove(SpherePack *pack);
#if DEMO
	unsigned int GetColor(void);
#endif
	
	void FrustumTest(const Frustum &f,SpherePackCallback *callback);
	
		SpherePackCallback *callback);
	
	void RangeTest(const Vector3d &center,float radius,SpherePackCallback *callback);
	void PointTest2d(const Vector3d &center, SpherePackCallback *callback);
	
		SpherePack *sphere);
	
	virtual void RangeTestCallback(const Vector3d &p,float distance,SpherePack *sphere,ViewState state);
	virtual void PointTest2dCallback(const Vector3d &p, SpherePack *sphere,ViewState state);
	
	virtual void VisibilityCallback(const Frustum &f,SpherePack *sphere,ViewState state);
	
	
	void Reset(void);
	
private:
	
	
	SpherePackCallback *mCallback;
	
	
	
#if DEMO
#define MAXCOLORS 12
	int               mColorCount;
	unsigned int      mColors[MAXCOLORS];
#endif
	
};

void SpherePack::NewPos(const Vector3d &pos)
{
	
	if (mParent && !HasSpherePackFlag(SPF_INTEGRATE))
	{	
		
		{
			mFactory->AddRecompute(mParent);

			Unlink();
		}
	}
	
}

void SpherePack::NewPosRadius(const Vector3d &pos,float radius)
{
	mCenter = pos;
	
	if (mParent && !HasSpherePackFlag(SPF_INTEGRATE))
	{
		if (radius != GetRadius())
		{
			SetRadius(radius);
			ComputeBindingDistance(mParent);
		}
		
		mFactory->AddRecompute(mParent);

		float dist = DistanceSquared(mParent);
	
		if (dist >= mBindingDistance)
		{
			Unlink();
			mFactory->AddIntegrate(this);
		}
	}
}

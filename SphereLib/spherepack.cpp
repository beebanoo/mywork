

#include "Stdafx.h"
#include "spherepack.h"

#if DEMO
int PrintText(int x, int y, int color, char* output, ...);
int DrawLine(int x1, int y1, int x2, int y2, int color);
int DrawCircle(int locx, int locy, int radius, int color);
#endif

SpherePackFactory::SpherePackFactory(int maxspheres, float rootsize, float leafsize, float gravy)
{
	NANOBEGIN
	mMaxRootSize      = rootsize;
	mMaxLeafSize      = leafsize;
	mSuperSphereGravy = gravy;
	mIntegrate        = new SpherePackFifo(maxspheres);
	mRecompute        = new SpherePackFifo(maxspheres);
	
	
	Vector3d p(0,0,0);
	
	mRoot->Init(this,p,6553600,0, false);
	mRoot->SetSpherePackFlag(SpherePackFlag(SPF_SUPERSPHERE | SPF_ROOTNODE | SPF_ROOT_TREE));
	
#if DEMO
	mRoot->SetColor(0x00FFFFFF);
#endif
	
	mLeaf->Init(this,p,1638400,0,false);
	mLeaf->SetSpherePackFlag(SpherePackFlag(SPF_SUPERSPHERE | SPF_ROOTNODE | SPF_LEAF_TREE));
	
#if DEMO
	mLeaf->SetColor(0x00FFFFFF);
	mColorCount = 0;
	
	mColors[0]  = 0x00FF0000;
	mColors[1]  = 0x0000FF00;
	mColors[2]  = 0x000000FF;
	mColors[3]  = 0x00FFFF00;
	mColors[4]  = 0x00FF00FF;
	mColors[5]  = 0x0000FFFF;
	mColors[6]  = 0x00FF8080;
	mColors[7]  = 0x0000FF80;
	mColors[8]  = 0x000080FF;
	mColors[9]  = 0x00FFFF80;
	mColors[10] = 0x00FF80FF;
	mColors[11] = 0x0080FFFF;
	
#endif
	NANOEND
}

SpherePackFactory::~SpherePackFactory(void)
{
}

void SpherePackFactory::Process(void)
{
	{
		int maxrecompute = mRecompute->GetCount();
		for (int i = 0; i < maxrecompute; ++i)
		{
			SpherePack * pack = mRecompute->Pop();
			if (!pack) break;
			bool kill = pack->Recompute(mSuperSphereGravy);
			if (kill) Remove(pack);
		}
	}
	
	{
		int maxintegrate = mIntegrate->GetCount();
		
		for (int i = 0; i < maxintegrate; ++i)
		{
			SpherePack * pack = mIntegrate->Pop();
			if (!pack)
				break;
			pack->SetFifo2(0);
			
			if (pack->HasSpherePackFlag(SPF_ROOT_TREE))
			else
		}
	}
	
}


SpherePack * SpherePackFactory::AddSphere_(const Vector3d &pos,
                                          float radius,
                                          void *userdata,
										  bool isSphere,
                                          int flags)
{
	
	SpherePack *pack = mSpheres.GetFreeLink();
	
	assert(pack);
	
	if (pack)
	{
		if (flags & SPF_ROOT_TREE)
		{
			pack->Init(this,pos,radius,userdata, isSphere);
		}
		else
		{
			pack->Init(this,pos,radius,userdata, isSphere);
		}
	}
	
	return pack;
}

void SpherePackFactory::AddIntegrate(SpherePack *pack)
{
	if (pack->HasSpherePackFlag(SPF_ROOT_TREE))
		mRoot->AddChild(pack);
	else
		mLeaf->AddChild(pack);
	
	pack->SetFifo2(fifo);
}

void SpherePackFactory::AddRecompute(SpherePack *recompute)
{
	if (!recompute->HasSpherePackFlag(SPF_RECOMPUTE))
	{
		if (recompute->GetChildCount())
		{
			SpherePack **fifo = mRecompute->Push(recompute);
			recompute->SetFifo1(fifo);
		}
		else
		{
			Remove(recompute);
		}
	}
}

void SpherePackFactory::Render(void)
{
#if DEMO
	mRoot->Render(mRoot->GetColor());
	mLeaf->Render(mLeaf->GetColor());
#endif
}


void SpherePack::Render(unsigned int )
{
#if DEMO
	if (!HasSpherePackFlag(SPF_ROOTNODE))
	{
		
		if (HasSpherePackFlag(SPF_SUPERSPHERE))
		{
			color = mColor;
		}
		else
		{
			if (mParent->HasSpherePackFlag(SPF_ROOTNODE)) color = 0x00FFFFFF;
		}
#if DEMO
		DrawCircle(int(mCenter.x), int(mCenter.y),int(GetRadius()),color);
#endif
		if (HasSpherePackFlag(SPF_SUPERSPHERE))
		{
			if (HasSpherePackFlag(SPF_LEAF_TREE))
			{
				
#if DEMO
				DrawCircle(int(mCenter.x), int(mCenter.y),int(GetRadius()),color);
#endif
#ifdef SPHERELIB_STRICT
		if (!sphere->IS_SPHERE)
			puts("SpherePack::Render");
#endif
				SpherePack *link = (SpherePack *) GetUserData();
				
				link = link->GetParent();
				
				if (link && !link->HasSpherePackFlag(SPF_ROOTNODE))
				{
					DrawLine(int(mCenter.x), int(mCenter.y),
						int(link->mCenter.x), int(link->mCenter.y),
						link->GetColor());
				}
			}
			else
			{
#if DEMO
				DrawCircle(int(mCenter.x), int(mCenter.y),int(GetRadius())+3,color);
#endif
			}
			
		}
		
	}
	
	if (mChildren)
	{
		SpherePack *pack = mChildren;
		
		while (pack)
		{
			pack->Render(color);
			pack = pack->_GetNextSibling();
		}
	}
#endif
}

bool SpherePack::Recompute(float gravy)
{
	
#if 1
	Vector3d total(0,0,0);
	int count=0;
	SpherePack *pack = mChildren;
	while (pack)
	{
		total+=pack->mCenter;
		count++;
		pack = pack->_GetNextSibling();
	}
	
	if (count)
	{
		float recip = 1.0f / float(count);
		total*=recip;
		
		Vector3d oldpos = mCenter;
		
#ifdef __STATIC_RANGE__
		assert(total.IsInStaticRange());
#endif
		float maxradius = 0;
		
		pack = mChildren;
		
		while (pack)
		{
			float dist = DistanceSquared(pack);
			float radius = sqrtf(dist) + pack->GetRadius();
			if (radius > maxradius)
			{
				maxradius = radius;
				if ((maxradius+gravy) >= GetRadius())
				{
#ifdef __STATIC_RANGE__
					assert(oldpos.IsInStaticRange());
#endif
					mCenter = oldpos;
					ClearSpherePackFlag(SPF_RECOMPUTE);
					return false;
				}
			}
			pack = pack->_GetNextSibling();
		}
		
		maxradius+=gravy;
		
		SetRadius(maxradius);
		
		pack = mChildren;
		
		while (pack)
		{
			pack->ComputeBindingDistance(this);
			pack = pack->_GetNextSibling();
		}
		
	}
	
#endif
	
	ClearSpherePackFlag(SPF_RECOMPUTE);
	
	return false;
}


void SpherePack::LostChild(SpherePack *t)
{
	assert(mChildCount);
	assert(mChildren);
	
	
	SpherePack *pack = mChildren;
	bool found = false;
	while (pack)
	{
		if (pack == t)
		{
			assert(!found);
			found = true;
		}
		pack = pack->_GetNextSibling();
	}
	assert(found);
	
#endif
	
	SpherePack *prev = t->_GetPrevSibling();
	
	if (prev)
	{
		SpherePack *next = t->_GetNextSibling();
		if (next) next->SetPrevSibling(prev);
	}
	else
	{
		SpherePack *next = t->_GetNextSibling();
		mChildren = next;
		if (mChildren) mChildren->SetPrevSibling(0);
	}
	
	mChildCount--;
	
	if (!mChildCount && HasSpherePackFlag(SPF_SUPERSPHERE))
	{
		mFactory->Remove(this);
	}
}

void SpherePackFactory::Remove(SpherePack*pack)
{
	
	
	if (pack->HasSpherePackFlag(SPF_SUPERSPHERE) && pack->HasSpherePackFlag(SPF_LEAF_TREE))
	{
#ifdef SPHERELIB_STRICT
		if (!pack->IS_SPHERE)
			puts("SpherePackFactory::Remove");
#endif
		SpherePack *link = (SpherePack *) pack->GetUserData();
		
		Remove(link);
	}
	
	pack->Unlink();
	
	mSpheres.Release(pack);
}

#if DEMO
unsigned int SpherePackFactory::GetColor(void)
{
	unsigned int ret = mColors[mColorCount];
	mColorCount++;
	if (mColorCount == MAXCOLORS) mColorCount = 0;
	return ret;
}
#endif

void SpherePackFactory::Integrate(SpherePack *pack,
                                  SpherePack *supersphere,
                                  float node_size)
{
	
	SpherePack *search = supersphere->GetChildren();
	
	
	
	
	while (search)
	{
		if (search->HasSpherePackFlag(SPF_SUPERSPHERE) && !search->HasSpherePackFlag(SPF_ROOTNODE) && search->GetChildCount())
		{
			
			float dist = pack->DistanceSquared(search);
			
			if (nearest1)
			{
				if (dist < neardist1)
				{
					
					float d = sqrtf(dist)+pack->GetRadius();
					
					if (d <= search->GetRadius())
					{
						neardist1 = dist;
						nearest1  = search;
					}
				}
			}
			else
			{
				
				float d = (sqrtf(dist) + pack->GetRadius())-search->GetRadius();
				
				if (d < neardist2)
				{
					if (d < 0)
					{
						neardist1 = dist;
						nearest1  = search;
					}
					else
					{
						neardist2 = d;
						nearest2  = search;
					}
				}
			}
		}
		search = search->_GetNextSibling();
	}
	
	if (nearest1)
	{
		pack->Unlink();
		nearest1->AddChild(pack);
		pack->ComputeBindingDistance(nearest1);
		nearest1->Recompute(mSuperSphereGravy);
		
		if (nearest1->HasSpherePackFlag(SPF_LEAF_TREE))
		{
#ifdef SPHERELIB_STRICT
		if (!nearest1->IS_SPHERE)
			puts("SpherePackFactory::Integrate1");
#endif
			SpherePack *link = (SpherePack *) nearest1->GetUserData();
			link->NewPosRadius(nearest1->GetPos(), nearest1->GetRadius());
		}
		
	}
	else
	{
		bool newsphere = true;
		
		if (nearest2)
		{
			float newsize = neardist2 + nearest2->GetRadius() + mSuperSphereGravy;
			
			if (newsize <= node_size)
			{
				pack->Unlink();
				
				nearest2->SetRadius(newsize);
				nearest2->AddChild(pack);
				nearest2->Recompute(mSuperSphereGravy);
				pack->ComputeBindingDistance(nearest2);
				
				if (nearest2->HasSpherePackFlag(SPF_LEAF_TREE))
				{
#ifdef SPHERELIB_STRICT
				if (!nearest2->IS_SPHERE)
					puts("SpherePackFactory::Integrate2");
#endif
					SpherePack *link = (SpherePack *) nearest2->GetUserData();
					link->NewPosRadius(nearest2->GetPos(), nearest2->GetRadius());
				}
				
				newsphere = false;
				
			}
			
		}
		
		if (newsphere)
		{
			assert(supersphere->HasSpherePackFlag(SPF_ROOTNODE));
			pack->Unlink();
			
			SpherePack *parent = mSpheres.GetFreeLink();
			assert(parent);
			parent->Init(this, pack->GetPos(), pack->GetRadius()+mSuperSphereGravy, 0, false);
			
			if (supersphere->HasSpherePackFlag(SPF_ROOT_TREE))
				parent->SetSpherePackFlag(SPF_ROOT_TREE);
			else
				parent->SetSpherePackFlag(SPF_LEAF_TREE);
			
			parent->SetSpherePackFlag(SPF_SUPERSPHERE);
#if DEMO
			parent->SetColor(GetColor());
#endif
			parent->AddChild(pack);
			
			supersphere->AddChild(parent);
			
			parent->Recompute(mSuperSphereGravy);
			pack->ComputeBindingDistance(parent);
			
			if (parent->HasSpherePackFlag(SPF_LEAF_TREE))
			{
				SpherePack *link = AddSphere_(parent->GetPos(), parent->GetRadius(), parent, true, SPF_ROOT_TREE);
			}
			
		}
	}
	
}


void SpherePackFactory::FrustumTest(const Frustum &f,SpherePackCallback *callback)
{
	mCallback = callback;
	mRoot->VisibilityTest(f,this,VS_PARTIAL);
}


void SpherePack::VisibilityTest(const Frustum &f,SpherePackCallback *callback,ViewState state)
{
	
	if (state == VS_PARTIAL)
	{
		state = f.ViewVolumeTest(mCenter, GetRadius());
#if DEMO
		if (state != VS_OUTSIDE)
		{
			DrawCircle(int(mCenter.x), int(mCenter.y), int(GetRadius()), 0x404040);
		}
#endif
	}
	
	if (HasSpherePackFlag(SPF_SUPERSPHERE))
	{
		
		
		if (state == VS_OUTSIDE)
		{
			ClearSpherePackFlag(SpherePackFlag(SPF_INSIDE | SPF_PARTIAL));
			SetSpherePackFlag(SPF_HIDDEN);
		}
		else
		{
			if (state == VS_INSIDE)
			{
				ClearSpherePackFlag(SpherePackFlag(SPF_PARTIAL | SPF_HIDDEN));
				SetSpherePackFlag(SPF_INSIDE);
			}
			else
			{
				ClearSpherePackFlag(SpherePackFlag(SPF_HIDDEN | SPF_INSIDE));
				SetSpherePackFlag(SPF_PARTIAL);
			}
		}
		
		SpherePack *pack = mChildren;
		
		while (pack)
		{
			pack->VisibilityTest(f,callback,state);
			pack = pack->_GetNextSibling();
		}
		
	}
	else
	{
		switch (state)
		{
		case VS_INSIDE:
			if (!HasSpherePackFlag(SPF_INSIDE))
			{
				ClearSpherePackFlag(SpherePackFlag(SPF_HIDDEN | SPF_PARTIAL));
				SetSpherePackFlag(SPF_INSIDE);
				callback->VisibilityCallback(f,this,state);
			}
			break;
		case VS_OUTSIDE:
			if (!HasSpherePackFlag(SPF_HIDDEN))
			{
				ClearSpherePackFlag(SpherePackFlag(SPF_INSIDE | SPF_PARTIAL));
				SetSpherePackFlag(SPF_HIDDEN);
				callback->VisibilityCallback(f,this,state);
			}
			break;
		case VS_PARTIAL:
			{
				ClearSpherePackFlag(SpherePackFlag(SPF_INSIDE | SPF_HIDDEN));
				SetSpherePackFlag(SPF_PARTIAL);
				callback->VisibilityCallback(f,this,state);
			}
			break;
		}
		
	}
}

void SpherePackFactory::RayTrace(const Vector3d &p1,
                                 const Vector3d &p2,
                                 SpherePackCallback *callback)
{
	Vector3d dir = p2;
	float dist = dir.Normalize();
	mCallback = callback;
	mRoot->RayTrace(p1,dir,dist,this);
}

#include "../EterBase/Debug.h"

void SpherePackFactory::RangeTest(const Vector3d &center,float radius,SpherePackCallback *callback)
{
#ifdef __STATIC_RANGE__
	if (!center.IsInStaticRange())
	{
		TraceError("SpherePackFactory::RangeTest - RANGE ERROR %f, %f, %f",
			center.x, center.y, center.z);
		assert("SpherePackFactory::RangeTest - RANGE ERROR");
		return;
	}
#endif
	mCallback = callback;
	mRoot->RangeTest(center,radius,this,VS_PARTIAL);
}

void SpherePackFactory::PointTest2d(const Vector3d &center, SpherePackCallback *callback)
{
#ifdef __STATIC_RANGE__
	if (!center.IsInStaticRange())
	{
		TraceError("SpherePackFactory::RangeTest2d - RANGE ERROR %f, %f, %f",
			center.x, center.y, center.z);
		assert("SpherePackFactory::RangeTest2d - RANGE ERROR");
		return;
	}
#endif
	mCallback = callback;

#ifdef SPHERELIB_STRICT
	mRoot->PointTest2d(center, this,VS_PARTIAL);
	extern bool MAPOUTDOOR_GET_HEIGHT_TRACE;
	if (MAPOUTDOOR_GET_HEIGHT_TRACE)
		puts("================================================");
#else
	mRoot->PointTest2d(center, this,VS_PARTIAL);
	
#endif
	
}

void SpherePack::RangeTest(const Vector3d &p,
                           float distance,
                           SpherePackCallback *callback,
                           ViewState state)
{	

	if (state == VS_PARTIAL)
	{
		float d = p.Distance(mCenter);
		if ((d-distance) > GetRadius()) return;;
		if ((GetRadius()+d) < distance) state = VS_INSIDE;
	}
	
	if (HasSpherePackFlag(SPF_SUPERSPHERE))
	{
#if DEMO
		if (state == VS_PARTIAL)
		{
			DrawCircle(int(mCenter.x), int(mCenter.y), int(GetRadius()), 0x404040);
		}
#endif
		SpherePack *pack = mChildren;
		while (pack)
		{
			pack->RangeTest(p,distance,callback,state);
			pack = pack->_GetNextSibling();
		}
		
	}
	else
	{		
		callback->RangeTestCallback(p,distance,this,state);
	}
}

void SpherePack::PointTest2d(const Vector3d &p,
                           SpherePackCallback *callback,
                           ViewState state)
{
	if (state == VS_PARTIAL)
	{
		float dx=p.x-mCenter.x;
		float dy=p.y-mCenter.y;
		float distSquare = (dx*dx)+(dy*dy);

		if (distSquare > GetRadius2()) return;;
		if (GetRadius2() < -distSquare) state = VS_INSIDE;
	}
	
	if (HasSpherePackFlag(SPF_SUPERSPHERE))
	{
#if DEMO
		if (state == VS_PARTIAL)
		{
			DrawCircle(int(mCenter.x), int(mCenter.y), int(GetRadius()), 0x404040);
		}
#endif
		SpherePack *pack = mChildren;
		while (pack)
		{
			pack->PointTest2d(p, callback, state);
			pack = pack->_GetNextSibling();
		}
		
	}
	else
	{
#ifdef SPHERELIB_STRICT
		extern bool MAPOUTDOOR_GET_HEIGHT_TRACE;
		if (MAPOUTDOOR_GET_HEIGHT_TRACE)
		{
			float dx=p.x-mCenter.x;
			float dy=p.y-mCenter.y;
			float distSquare = (dx*dx)+(dy*dy);
			printf("--- (%f, %f) dist %f radius %f isSphere %d\n", mCenter.x, mCenter.y, distSquare, GetRadius(), IS_SPHERE);
		}
#endif
		callback->PointTest2dCallback(p, this, state);
	}
}

void SpherePackFactory::RangeTestCallback(const Vector3d &p,float distance,SpherePack *sphere,ViewState state)
{
#ifdef SPHERELIB_STRICT
	if (!sphere->IS_SPHERE)
		puts("SpherePackFactory::RangeTestCallback");
#endif
	SpherePack *link = (SpherePack *) sphere->GetUserData();
	if (link) link->RangeTest(p,distance,mCallback,state);
};

void SpherePackFactory::PointTest2dCallback(const Vector3d &p, SpherePack *sphere,ViewState state)
{
#ifdef SPHERELIB_STRICT
	if (!sphere->IS_SPHERE)
		puts("SpherePackFactory::PointTest2dCallback");
#endif
	SpherePack *link = (SpherePack *) sphere->GetUserData();
	if (link) link->PointTest2d(p, mCallback,state);
};

void SpherePack::RayTrace(const Vector3d &p1,
                          const Vector3d &dir,
                          float distance,
                          SpherePackCallback *callback)
{
	bool hit = false;
	
	if (HasSpherePackFlag(SPF_SUPERSPHERE))
	{
		
		hit = RayIntersectionInFront(p1,dir,0);
		
		if (hit)
		{
#if DEMO
			DrawCircle(int(mCenter.x), int(mCenter.y), int(GetRadius()), 0x404040);
#endif
			SpherePack *pack = mChildren;
			
			while (pack)
			{
				pack->RayTrace(p1,dir,distance,callback);
				pack = pack->_GetNextSibling();
			}
		}
		
	}
	else
	{
		Vector3d sect;
		hit = RayIntersection(p1,dir,distance,&sect);
		if (hit)
		{
			callback->RayTraceCallback(p1,dir,distance,sect,this);
		}
	}
}

										 SpherePack *sphere)
{
#ifdef SPHERELIB_STRICT
	if (!sphere->IS_SPHERE)
		puts("SpherePackFactory::RayTraceCallback");
#endif
	SpherePack *link = (SpherePack *) sphere->GetUserData();
	if (link) link->RayTrace(p1,dir,distance,mCallback);
};




void SpherePackFactory::Reset(void)
{
	mRoot->Reset();
	mLeaf->Reset();
}


void SpherePack::Reset(void)
{
	ClearSpherePackFlag(SpherePackFlag(SPF_HIDDEN | SPF_PARTIAL | SPF_INSIDE));
	
	SpherePack *pack = mChildren;
	while (pack)
	{
		pack->Reset();
		pack = pack->_GetNextSibling();
	}
}

void SpherePackFactory::VisibilityCallback(const Frustum &f,SpherePack *sphere,ViewState state)
{
#ifdef SPHERELIB_STRICT
	if (!sphere->IS_SPHERE)
		puts("SpherePackFactory::VisibilityCallback");
#endif
	SpherePack *link = (SpherePack *) sphere->GetUserData();
	if (link) link->VisibilityTest(f,mCallback,state);
}

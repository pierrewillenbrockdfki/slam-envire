#include <Eigen/Core>
#include <Eigen/LU>

#include "envire/Core.hpp"
#include "envire/maps/LaserScan.hpp"
#include "envire/maps/TriMesh.hpp"
#include "envire/operators/ScanMeshing.hpp"

#include "envire/core/Event.hpp"
#include "envire/core/EventHandler.hpp"

#define BOOST_TEST_MODULE EnvireTest 
#include <boost/test/included/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "envire/tools/GridAccess.hpp"
#include "envire/maps/Grids.hpp"

#include "envire/maps/MultiLevelSurfaceGrid.hpp"
#include "envire/operators/MLSProjection.hpp"
#include "envire/operators/MergeMLS.hpp"

#include "base/timemark.h"
   
using namespace envire;
using namespace std;

template<class T> bool contains(const std::list<T>& list, const T& element)
{
    return find( list.begin(), list.end(), element ) != list.end();
};

class DummyOperator : public Operator 
{
public:
    void set( EnvironmentItem* other ) {}
    Operator* clone() const {return new DummyOperator(*this);}
    bool updateAll() { return true; };
    void serialize(Serialization &) {};
};

class DummyLayer : public Layer 
{
public:
    void set( EnvironmentItem* other ) {}
    Layer* clone() const {return new DummyLayer(*this);};
    void serialize(Serialization &) {};
};

class DummyCartesianMap : public Map<2> 
{
public:
    void set( EnvironmentItem* other ) {}
    CartesianMap* clone() const {return new DummyCartesianMap(*this);};
    void serialize(Serialization &) {};
    Extents getExtents() const { return Extents(); }
};

BOOST_AUTO_TEST_CASE( EnvironmentItem_test )
{
    MergeMLS merge;
    BOOST_CHECK_EQUAL( merge.getClassName(), "envire::MergeMLS" );
}

BOOST_AUTO_TEST_CASE( TreeTest )
{
    // set up an environment
    boost::scoped_ptr<Environment> env( new Environment() );

    //check if three nodes are the childs of root
    // create some child framenodes
    FrameNode *fn1, *fn2, *fn3;
    fn1 = new FrameNode();
    fn1->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d( 0.0, 0.0, 0.5 )) );
    fn2 = new FrameNode();
    fn2->setTransform( 
	    Eigen::Transform3d(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0 )));
    fn3 = new FrameNode();

    //attach 3 node to root
    env->addChild(env->getRootNode(), fn1);
    env->addChild(env->getRootNode(), fn2);
    env->addChild(env->getRootNode(), fn3);
    
    std::list<FrameNode *> childs = env->getChildren(env->getRootNode());
    
    BOOST_CHECK( contains(env->getChildren(env->getRootNode()),fn1) );
    BOOST_CHECK( contains(env->getChildren(env->getRootNode()),fn2) );
    BOOST_CHECK( contains(env->getChildren(env->getRootNode()),fn3) );
    
    //attach fn2 to fn3
    //this will of course detach fn2 from the rootnode and attach it to fn3
    env->addChild(fn3, fn2);
    
    BOOST_CHECK( contains(env->getChildren(fn3) ,fn2) );
    
    //remove fn2 from rootNode
    env->removeChild(env->getRootNode(), fn2);
    BOOST_CHECK( contains(env->getChildren(fn3) ,fn2) );
    BOOST_CHECK(!contains(env->getChildren(env->getRootNode()) ,fn2) );
}

BOOST_AUTO_TEST_CASE( environment )
{
    // set up an environment
    boost::scoped_ptr<Environment> env( new Environment() );

    // an environment should always have a root node 
    BOOST_CHECK( env->getRootNode() );

    // create some child framenodes
    FrameNode *fn1, *fn2, *fn3;
    fn1 = new FrameNode();
    fn1->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d( 0.0, 0.0, 0.5 )) );
    fn2 = new FrameNode();
    fn2->setTransform( 
	    Eigen::Transform3d(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0 )));
    fn3 = new FrameNode();
    fn3->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d(0.0, 1.0, 0.0)));
    
    // attach explicitely
    env->attachItem( fn1 );
    BOOST_CHECK( fn1->isAttached() );
    BOOST_CHECK_EQUAL( fn1->getEnvironment(), env.get() );
    env->addChild( env->getRootNode(), fn1 );
    BOOST_CHECK_EQUAL( env->getRootNode(), env->getParent(fn1) );
    BOOST_CHECK( contains(env->getChildren(env->getRootNode()),fn1) );

    // implicit attachment
    env->addChild( fn1, fn2 );
    BOOST_CHECK( fn2->isAttached() );
    
    // setup the rest of the framenodes
    env->addChild( env->getRootNode(), fn3 );    

    // perform a relative transformation
    FrameNode::TransformType rt1 = env->relativeTransform(fn2, fn1);
    BOOST_CHECK( rt1.matrix().isApprox( fn2->getTransform().matrix(), 1e-10 ) );

    FrameNode::TransformType rt2 = env->relativeTransform(fn2, fn3);
    BOOST_CHECK( rt2.matrix().isApprox( 
		fn3->getTransform().inverse() * fn1->getTransform() * fn2->getTransform(),
		1e-10 ) );
    
    // now do the same for layers
    Layer *l1, *l2, *l3;
    l1 = new DummyLayer();
    l2 = new DummyLayer();
    l3 = new DummyLayer();

    env->attachItem( l1 );
    BOOST_CHECK( l1->isAttached() );
    BOOST_CHECK_EQUAL( l1->getEnvironment(), env.get() );

    env->addChild( l1, l2 );
    BOOST_CHECK_EQUAL( l1, env->getParent(l2) );
    BOOST_CHECK( contains(env->getChildren(l1),l2) );

    env->attachItem(l3);

    // CartesianMaps should work the same
    CartesianMap *m1, *m2;
    m1 = new DummyCartesianMap();
    m2 = new DummyCartesianMap();

    env->attachItem( m1 );
    env->attachItem( m2 );

    env->setFrameNode( m1, fn1 );
    env->setFrameNode( m2, fn1 );

    BOOST_CHECK_EQUAL( env->getFrameNode( m1 ), fn1 );
    BOOST_CHECK( contains(env->getMaps(fn1),m1) );
    BOOST_CHECK( contains(env->getMaps(fn1),m2) );
    
    // now to operators
    Operator *o1;
    o1 = new DummyOperator();
    env->attachItem( o1 );

    env->addInput( o1, l1 );
    env->addInput( o1, l2 );
    env->addOutput( o1, l3 );

    BOOST_CHECK( contains(env->getInputs(o1),l1) );
    BOOST_CHECK( contains(env->getOutputs(o1),l3) );
}

BOOST_AUTO_TEST_CASE( serialization )
{
    Serialization so;
    boost::scoped_ptr<Environment> env( new Environment() );

    // create some child framenodes
    FrameNode *fn1, *fn2, *fn3;
    fn1 = new FrameNode();
    fn1->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d( 0.0, 0.0, 0.5 )) );
    fn2 = new FrameNode();
    fn2->setTransform( 
	    Eigen::Transform3d(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0 )));
    fn3 = new FrameNode();
    
    // attach explicitely
    env->attachItem( fn1 );
    env->addChild( env->getRootNode(), fn1 );
    env->addChild( fn1, fn2 );
    env->addChild( env->getRootNode(), fn3 );

    // TODO get cmake to somehow add an absolute path here
    std::string path("build/test");
    so.serialize(env.get(), path);

    // now try to parse the thing again
    boost::scoped_ptr<Environment> env2(so.unserialize( "build/test" ));

    // TODO check that the structure is the same
}

BOOST_AUTO_TEST_CASE( functional ) 
{
    boost::scoped_ptr<Environment> env( new Environment() );

    LaserScan* scan = LaserScan::importScanFile("test/test.scan", env->getRootNode() );

    // create a TriMesh Layer and attach it to the root Node.
    TriMesh* mesh = new TriMesh();
    env->attachItem( mesh );

    // set up a meshing operator on the output mesh. Add then an input
    // and parametrize the meshing operation. 
    ScanMeshing* mop = new ScanMeshing();
    env->attachItem( mop );

    mop->setMaxEdgeLength(0.5);

    mop->addInput(scan);
    mop->addOutput(mesh);

    mop->updateAll();

    Serialization so;
    so.serialize(env.get(), "build/test");
}

BOOST_AUTO_TEST_CASE( grid_access ) 
{
    boost::scoped_ptr<Environment> env( new Environment() );
    ElevationGrid *m1 = new ElevationGrid( 2, 4, 1, 1 );
    ElevationGrid *m2 = new ElevationGrid( 4, 2, 1, 1 );

    env->attachItem( m1 );
    env->attachItem( m2 );

    FrameNode *fn1 = new FrameNode();
    fn1->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d( 0.0, 0.0, 0.0 )) );
    FrameNode *fn2 = new FrameNode();
    fn2->setTransform( 
	    Eigen::Transform3d(Eigen::Translation3d( 5.0, 0.0, 0.0 )) );

    env->addChild( env->getRootNode(), fn1 );
    env->addChild( env->getRootNode(), fn2 );

    m1->setFrameNode( fn1 );
    m2->setFrameNode( fn2 );

    for(int i=0;i<2;i++)
    {
	for(int j=0;j<4;j++)
	{
	    m1->getGridData()[j][i] = i*4 + j;
	    m2->getGridData()[i][j] = i*4 + j;
	}
    }

    std::vector<Eigen::Vector3d> probes;
    probes.push_back( Eigen::Vector3d(0.5,0.5,0) );
    probes.push_back( Eigen::Vector3d(1.5,3.5,0) );
    probes.push_back( Eigen::Vector3d(4.5,0.5,0) );
    probes.push_back( Eigen::Vector3d(6.5,0.5,0) );
    probes.push_back( Eigen::Vector3d(7.5,1.5,0) );

    GridAccess ga( env.get() );

    for(size_t i=0;i<probes.size();i++)
    {
	bool r;
        r = ga.getElevation( probes[i] );
	// TODO: actually check the values here
    }
}

BOOST_AUTO_TEST_CASE( pointcloud_access ) 
{
    boost::scoped_ptr<Environment> env( new Environment() );
    Pointcloud *pc = new Pointcloud();

    for(int i=0;i<500;i++)
    {
	pc->vertices.push_back( Eigen::Vector3d::Random() );
    }
    env->attachItem( pc );
    env->setFrameNode( pc, env->getRootNode() );

    PointcloudAccess pa( env.get() );

    bool success;
    Eigen::Vector3d v;

    base::TimeMark a("dist");
    for(int i=0;i<10000;i++)
    {
	v = Eigen::Vector3d::Random();
	success = pa.getElevation( v, 0.1 );
    }
    cout << a << endl;;

    base::TimeMark b("block");
    for(int i=0;i<10000;i++)
    {
	v = Eigen::Vector3d::Random();
	success = pa.getElevation( v, 0.1, 0, 0.2 );
    }
    cout << b << endl;
}

BOOST_AUTO_TEST_CASE( multilevelsurfacegrid ) 
{
    boost::scoped_ptr<Environment> env( new Environment() );
    MultiLevelSurfaceGrid *mls = new MultiLevelSurfaceGrid(10, 10, 0.1, 0.1);
    env->attachItem( mls );

    mls->insertHead( 0,0, MultiLevelSurfaceGrid::SurfacePatch( 1.0, 0.1, 0, true ) );
    mls->insertHead( 0,0, MultiLevelSurfaceGrid::SurfacePatch( 2.0, 0.1, 0.5, false ) );

    mls->insertHead( 2,1, MultiLevelSurfaceGrid::SurfacePatch( 3.0, 0.1, 0.5, false ) );

    MultiLevelSurfaceGrid::iterator it = mls->beginCell(0,0);
    BOOST_CHECK_EQUAL( it->mean, 2.0 );
    it++;
    BOOST_CHECK_EQUAL( (*it).mean, 1.0 );
    ++it;
    BOOST_CHECK_EQUAL( it, mls->endCell() );

    MultiLevelSurfaceGrid::iterator it2 = mls->beginCell(2,1);
    BOOST_CHECK_EQUAL( it2->mean, 3.0 );
    it2++;

    Serialization so;
    so.serialize(env.get(), "build/test");

    boost::scoped_ptr<Environment> env2(so.unserialize( "build/test" ));
    MultiLevelSurfaceGrid *mls2 = env2->getItems<MultiLevelSurfaceGrid>().front();

    MultiLevelSurfaceGrid::iterator it3 = mls2->beginCell(0,0);
    BOOST_CHECK_EQUAL( it3->mean, 2.0 );
    it3++;
    BOOST_CHECK_EQUAL( (*it3).mean, 1.0 );
    ++it3;
    BOOST_CHECK_EQUAL( it3, mls->endCell() );
}

BOOST_AUTO_TEST_CASE( env_eventsync ) 
{
    boost::scoped_ptr<Environment> env( new Environment() );
    boost::scoped_ptr<Environment> env2( new Environment() );

    // the eventprocessor will queue events until flush is called,
    // then all events are applied to the given environment
    EventProcessor ep( env2.get() );
    env->addEventHandler( &ep );

    // create some child framenodes
    FrameNode *fn1, *fn2, *fn3;
    fn1 = new FrameNode();
    Eigen::Transform3d t1(Eigen::Translation3d( 0.0, 0.0, 0.5 ));
    Eigen::Transform3d t2(Eigen::Translation3d( 0.0, 0.0, 1.5 ));
    Eigen::Transform3d t3(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0 ));

    fn1->setTransform( t1 );
    fn2 = new FrameNode();
    fn2->setTransform( t2 ); 
    fn3 = new FrameNode();
    fn3->setTransform( t3 );
    
    // attach explicitely
    env->attachItem( fn1 );
    env->addChild( env->getRootNode(), fn1 );
    env->addChild( fn1, fn2 );
    env->addChild( env->getRootNode(), fn3 );

    ep.flush();
    BOOST_CHECK( env2->getItem( fn1->getUniqueId() ) );
    BOOST_CHECK_EQUAL( 
	    env2->getItem<FrameNode>( fn1->getUniqueId() )->getTransform().translation().z(), 
	    fn1->getTransform().translation().z()
	    );

    fn1->setTransform( t2 );
    ep.flush();
    BOOST_CHECK_EQUAL( 
	    env2->getItem<FrameNode>( fn1->getUniqueId() )->getTransform().translation().z(), 
	    fn1->getTransform().translation().z()
	    );

    env->removeEventHandler( &ep );
}

BOOST_AUTO_TEST_CASE( mlsprojection_test ) 
{
    // set up test environment
    boost::scoped_ptr<Environment> env( new Environment() );

    MultiLevelSurfaceGrid *mls = new MultiLevelSurfaceGrid(100, 100, 0.1, 0.1);
    env->attachItem( mls );

    FrameNode *fm = new FrameNode( Eigen::Transform3d( Eigen::Translation3d( -5, -5, 0 ) ) );
    env->getRootNode()->addChild( fm );
    mls->setFrameNode( fm );

    envire::Pointcloud* pc = new envire::Pointcloud();
    env->attachItem( pc );
    std::vector<double> &vars = pc->getVertexData<double>( Pointcloud::VERTEX_VARIANCE );
    for( int i=0; i<100; i++ )
    {
	const double r = i/100.0;
	pc->vertices.push_back( Eigen::Vector3d( r-0.5, 1.0, 0 ) );
	vars.push_back( 0 );
    }

    FrameNode *pcfm = new FrameNode( Eigen::Transform3d( Eigen::Translation3d( 0, 0, 0 ) ) );
    env->getRootNode()->addChild( pcfm );
    pc->setFrameNode( pcfm );

    envire::MLSProjection *proj = new envire::MLSProjection();
    env->attachItem( proj );
    proj->addInput( pc );
    proj->addOutput( mls );
    proj->useUncertainty( true );

    for(int i=0;i<500;i++)
    {
	double r = i*0.1;
	Eigen::Matrix<double,6,1> c;
	//c << 0, M_PI/8.0, 0, 0, 0, 0;
	c << 0, r*0.1, 0, 0, 0, 0;
	pcfm->setTransform( TransformWithUncertainty( 
		    Eigen::Transform3d( Eigen::Translation3d( 0, r+0.05, 0 ) ),
		   c.cwise().square().asDiagonal() ) );
	proj->updateAll();
    }
}

BOOST_AUTO_TEST_CASE( mlsmerge_test ) 
{
    // set up test environment
    boost::scoped_ptr<Environment> env( new Environment() );

    MultiLevelSurfaceGrid *mls1 = new MultiLevelSurfaceGrid(100, 100, 0.1, 0.1);
    env->attachItem( mls1 );

    MultiLevelSurfaceGrid *mls2 = new MultiLevelSurfaceGrid(100, 100, 0.1, 0.1);
    env->attachItem( mls2 );

    for( size_t m=0; m<100; m++ )
    {
	for( size_t n=0; n<100; n++ )
	{
	    double h1 = cos(n*0.1)*sin(n*0.1);
	    double h2 = cos(n*0.2)*sin(n*0.3);
	    mls1->insertHead( m, n, MultiLevelSurfaceGrid::SurfacePatch( h1, 0.1 ) );
	    mls2->insertHead( m, n, MultiLevelSurfaceGrid::SurfacePatch( h2, 0.1 ) );
	}
    }

    FrameNode *fm = new FrameNode( Eigen::Transform3d( Eigen::Translation3d( -5, -5, 0 ) ) );
    env->getRootNode()->addChild( fm );
    mls1->setFrameNode( fm );

    FrameNode *fm2 = new FrameNode( Eigen::Transform3d( Eigen::Translation3d( -5, -5, 1 ) ) );
    env->getRootNode()->addChild( fm2 );
    mls2->setFrameNode( fm2 );

    envire::MergeMLS *merge = new envire::MergeMLS();
    env->attachItem( merge );
    merge->addInput( mls1 );
    merge->addOutput( mls2 );
    merge->updateAll();

}
// EOF
//

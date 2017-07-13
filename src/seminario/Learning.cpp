#include "Learning.h"
#include <dlib/svm.h>
#include "csvParser.h"

using namespace dlib;
using seminario::Learning;

// Typedef de Data
typedef matrix < double > SingleSample;
typedef std::vector< SingleSample > SetSample;
typedef std::vector< double > SetLabels;
// Typedef de Normalizer
typedef vector_normalizer< SingleSample > SetNormalizer;
// Typedef de Kernels
typedef radial_basis_kernel<SingleSample> RadialKernel;
typedef sigmoid_kernel<SingleSample> SigmoidKernel;
typedef polynomial_kernel<SingleSample> PolinomialKernel;
typedef linear_kernel<SingleSample> LinearKernel;
// Typedefe de Trainer
template < typename kernel_type >
using SvmTrainer = svm_nu_trainer< kernel_type >;
// Typedef de Functiones de descision
template < typename kernel_type >
using DecisionFunction = decision_function < SvmTrainer< kernel_type > >;

template < typename kernel_type >
using FunctionType= normalized_function< DecisionFunction< kernel_type > >;

const SString Learning::filesInputDirectory = "/home/kevin/ddpc/seminarioJs/.meteor/local/build/programs/server/assets/app/uploads/Text/";
const SString Learning::filesOutputDirectory = "./savedFiles/";
std::unique_ptr< Learning > Learning::m_instance= nullptr;

Learning::Learning()
{}

Learning::~Learning()
{}

bool Learning::getFileAndExist( const Value& params , string& filename )
{
    if( !params.IsArray() )
    {
        cout << "params error format" << endl;
        return false;
    }
    if( static_cast< int >( params.Size() ) != 1 )
    {
        cout << "params error size" << endl;
        return false;
    }
    const Value& fileValue=params[ 0 ];
    if( !fileValue.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filename = fileValue.GetString();
    return true;
}

bool Learning::getFileAndExist( const Value& params , SString& filenameTrained , SString& filename )
{
    if( !params.IsArray() )
    {
        cout << "params error format" << endl;
        return false;
    }
    if( static_cast< int >( params.Size() ) != 2 )
    {
        cout << "params error size" << endl;
        return false;
    }
    const Value& fileValueTrained=params[ 0 ];
    if( !fileValueTrained.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filenameTrained = fileValueTrained.GetString();

    const Value& fileValue=params[ 1 ];
    if( !fileValue.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filename = fileValue.GetString();

    return true;
}

bool readSamples( const SString& filename , SetSample& setSample , SetLabels& setLabels , unsigned int& countFields , unsigned int& countSamples )
{
    csvParser csv;
    if( !csv.openFile( filename ) )
    {
        return false;
    }
    while( true )
    {
        SString line=csv.nextLine();
        if( line.empty() )
        {
            break;
        }
        countSamples++;
        auto tokens = csv.split( line , ' ' );
        countFields = tokens.size() - 1;
        SingleSample sample( countFields , 1 );
        for( int j = 0 ; j < countFields ; j++ )    
        {
            sample( j ) = stof( tokens[ j ] );
        }
        setSample.push_back( sample );
        const double label = stof( tokens[ countFields ] ) ;
        if( label == 0 )
        {
            setLabels.push_back( -1 );
        }
        else
        {
            setLabels.push_back( +1 );
        }
    }
    return true;   
}

bool readSample( const SString& filename , SingleSample& sample )
{
    csvParser csv;
    if( !csv.openFile( filename ) )
    {
        return false;
    }
    SString line = csv.nextLine();
    if( line.empty() )
    {
        return false;
    }
    auto tokens = csv.split( line , ' ' );
    const unsigned int countFields = tokens.size();
    SingleSample temp( countFields , 1 );
    for( int i=0 ; i< countFields ; i++ )    
    {
        temp( i ) = stof( tokens[ i ] );
    }
    sample = temp;
    return true;
}

template < typename kernel_type >
void trainRadialKernel( Writer< StringBuffer >& writer , SvmTrainer< kernel_type >& trainer , SetSample& samples , SetLabels& labels , double max_nu )
{
    writer.StartArray();
    for (double gamma = 0.01; gamma <= 1; gamma *= 5 )
    {
        for (double nu = 0.01; nu < max_nu; nu *= 5 )
        {
            trainer.set_kernel( kernel_type( gamma ) );
            trainer.set_nu(nu);
            cout << "gamma: " << gamma << "    nu: " << nu;
            auto result = cross_validate_trainer( trainer , samples , labels , 3 );
            cout << "    cross validation accuracy: "<< result;
            writer.Key("gamma");
            writer.Double( gamma );
            writer.Key("nu");
            writer.Double( nu );
            writer.StartArray();
            writer.Double( result( 0 ) );
            writer.Double( result( 1 ) );
            writer.EndArray();
        }
    }
    writer.EndArray();  
}


template < typename kernel_type >
void trainSigmoidKernel( Writer< StringBuffer >& writer , SvmTrainer< kernel_type >& trainer , SetSample& samples , SetLabels& labels , double max_nu )
{
    writer.StartArray();
    for (double gamma = 0.01; gamma <= 1; gamma *= 5 )
    {
        for (double nu = 0.01; nu < max_nu; nu *= 5 )
        {
            trainer.set_kernel( kernel_type( gamma , 1 ) );
            trainer.set_nu(nu);
            cout << "gamma: " << gamma << "    nu: " << nu;
            auto result=cross_validate_trainer( trainer , samples , labels , 3 );
            cout << "cross validation accuracy: " << result ;
            writer.Key("gamma");
            writer.Double( gamma );
            writer.Key("nu");
            writer.Double( nu );
            writer.StartArray();
            writer.Double( result( 0 ) );
            writer.Double( result( 1 ) );
            writer.EndArray();
        }
    }
    writer.EndArray();  
}


template < typename kernel_type >
void trainPolynomialKernel( Writer< StringBuffer >& writer , SvmTrainer< kernel_type >& trainer , SetSample& samples , SetLabels& labels , double max_nu )
{
    writer.StartArray();
    for (double gamma = 0.01; gamma <= 1; gamma *= 5 )
    {
        for (double nu = 0.01; nu < max_nu; nu *= 5 )
        {
            trainer.set_kernel( kernel_type( gamma , 1 , 2 ) );
            trainer.set_nu(nu);
            cout << "gamma: " << gamma << "    nu: " << nu;
            auto result=cross_validate_trainer( trainer , samples , labels , 3 );
            cout << "cross validation accuracy: " << result;
            writer.Key("gamma");
            writer.Double( gamma );
            writer.Key("nu");
            writer.Double( nu );
            writer.StartArray();
            writer.Double( result( 0 ) );
            writer.Double( result( 1 ) );
            writer.EndArray();
        }
    }
    writer.EndArray();  
}

template < typename kernel_type >
void trainLinearKernel( Writer< StringBuffer >& writer , SvmTrainer< kernel_type >& trainer , SetSample& samples , SetLabels& labels , double max_nu )
{
    writer.StartArray();
    for (double nu = 0.01; nu < max_nu; nu *= 5 )
    {
        trainer.set_kernel( kernel_type() );
        trainer.set_nu(nu);
        cout << "gamma: " << gamma << "    nu: " << nu;
        auto result= cross_validate_trainer( trainer , samples , labels , 3 );
        cout << "cross validation accuracy: " << result;
        writer.Key("nu");
        writer.Double( nu );
        writer.StartArray();
        writer.Double( result( 0 ) );
        writer.Double( result( 1 ) );
        writer.EndArray();
    }
    writer.EndArray();  
}

void Learning::train( const Value& params , SString& output )
{
    StringBuffer s;
    Writer< StringBuffer > writer( s );
    writer.StartObject();  
    SString filename;
    if( !getFileAndExist( params , filename ) )
    {
        writer.Key("error");           
        writer.String( "no file found" );
        return;
    }
    Document doc;
    SetSample setSample;
    SetLabels setLabels;
    unsigned int countFields = 0;
    unsigned int countSamples = 0;
    // leyendo data
    if ( !readSamples( filesInputDirectory + filename +".csv" , setSample , setLabels , countFields , countSamples ) )
    {
        writer.Key("error");           
        writer.String( "error reading file" );
        return;
    }
    writer.Key("countFeatures");           
    writer.Uint( countFields );
    writer.Key("countSample");           
    writer.Uint( countSamples );


    SetNormalizer normalizer;
    normalizer.train(setSample);
    for ( unsigned long i = 0; i < countSamples ; ++i)
    {
        setSample[i] = normalizer(setSample[i]); 
    }
    randomize_samples(setSample, setLabels);
    const double max_nu = maximum_nu(setLabels);
    cout << max_nu << endl;

    writer.Key("radial");          
    SvmTrainer< RadialKernel > trainerRadial;
    trainRadialKernel(writer, trainerRadial , setSample , setLabels , max_nu );
    
    writer.Key("sigmoid");   
    SvmTrainer< SigmoidKernel > trainerSigmoid;
    trainSigmoidKernel(writer, trainerSigmoid , setSample , setLabels , max_nu );

    writer.Key("lineal");    
    SvmTrainer< LinearKernel > trainerLineal;
    trainLinearKernel(writer, trainerLineal , setSample , setLabels , max_nu );

    writer.Key("polinomial");           
    SvmTrainer< PolinomialKernel > trainerPolinomial;
    trainPolynomialKernel(writer, trainerPolinomial , setSample , setLabels , max_nu );
    
    writer.EndObject();
    output = s.GetString();
}

void Learning::predict( const Value& params , string& output )
{
    SString filenameTrained;
    SString filename;
    if( !getFileAndExist( params , filenameTrained , filename ) )
    {
        output="{ error : \"no file\" }";
        return;
    }
    SingleSample sample;
    if ( !readSample( filesInputDirectory + filename +".csv" , sample ) )
    {
        output="{ error : \"file error\" }";
        return;
    }
    output="ok";
}

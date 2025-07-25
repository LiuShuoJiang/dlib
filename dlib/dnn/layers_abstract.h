// Copyright (C) 2015  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#undef DLIB_DNn_LAYERS_ABSTRACT_H_
#ifdef DLIB_DNn_LAYERS_ABSTRACT_H_

#include "../cuda/tensor_abstract.h"
#include "core_abstract.h"
#include "../cuda/operation_mode.h"


namespace dlib
{

// ----------------------------------------------------------------------------------------

    class SUBNET 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object represents a deep neural network.  In particular, it is
                the simplified interface through which layer objects interact with their
                subnetworks.  A layer's two important tasks are to (1) take outputs from its
                subnetwork and forward propagate them through itself and (2) to backwards
                propagate an error gradient through itself and onto its subnetwork.
                The idea of a subnetwork is illustrated in the following diagram:

                  +---------------------------------------------------------+
                  | loss <-- layer1 <-- layer2 <-- ... <-- layern <-- input |
                  +---------------------------------------------------------+
                                      ^                            ^
                                      \__ subnetwork for layer1 __/

                Therefore, by "subnetwork" we mean the part of the network closer to the
                input.

                Note that there is no dlib::SUBNET type.  It is shown here purely to
                document the interface layer objects expect to see when they interact
                with a network.
        !*/

    public:
        // You aren't allowed to copy subnetworks from inside a layer.
        SUBNET(const SUBNET&) = delete;
        SUBNET& operator=(const SUBNET&) = delete;

        const tensor& get_output(
        ) const;
        /*!
            ensures
                - returns the output of this subnetwork.  This is the data that the next
                  layer in the network will take as input.
                - have_same_dimensions(#get_gradient_input(), get_output()) == true
        !*/

        tensor& get_gradient_input(
        );
        /*!
            ensures
                - returns the error gradient for this subnetwork.  That is, this is the
                  error gradient that this network will use to update itself.  Therefore,
                  when performing back propagation, layers that sit on top of this
                  subnetwork write their back propagated error gradients into
                  get_gradient_input().  Or to put it another way, during back propagation,
                  layers take the contents of their get_gradient_input() and back propagate
                  it through themselves and store the results into their subnetwork's
                  get_gradient_input().
        !*/

        const NEXT_SUBNET& subnet(
        ) const;
        /*!
            ensures
                - returns the subnetwork of *this network.  With respect to the diagram
                  above, if *this was layer1 then subnet() would return the network that
                  begins with layer2.
        !*/

        NEXT_SUBNET& subnet(
        );
        /*!
            ensures
                - returns the subnetwork of *this network.  With respect to the diagram
                  above, if *this was layer1 then subnet() would return the network that
                  begins with layer2.
        !*/

        const INPUT_LAYER& input_layer(
        ) const;
        /*!
            ensures
                - returns the very first layer in *this network.  It's equivalent to calling
                  subnet() recursively until you get to the first layer.  This means it will return
                  the object that is an implementation of the EXAMPLE_INPUT_LAYER interface defined
                  in input_abstract.h
        !*/

        INPUT_LAYER& input_layer(
        );
        /*!
            ensures
                - returns the very first layer in *this network.  It's equivalent to calling
                  subnet() recursively until you get to the first layer.  This means it will return
                  the object that is an implementation of the EXAMPLE_INPUT_LAYER interface defined
                  in input_abstract.h
        !*/

        const layer_details_type& layer_details(
        ) const; 
        /*!
            ensures
                - returns the layer_details_type instance that defines the behavior of the
                  layer at the top of this network.  I.e. returns the layer details that
                  defines the behavior of the layer nearest to the network output rather
                  than the input layer.  For computational layers, this is the object
                  implementing the EXAMPLE_COMPUTATIONAL_LAYER_ interface that defines the
                  layer's behavior.
        !*/

        unsigned int sample_expansion_factor (
        ) const;
        /*!
            ensures
                - When to_tensor() is invoked on this network's input layer it converts N
                  input objects into M samples, all stored inside a resizable_tensor.  It
                  is always the case that M is some integer multiple of N.
                  sample_expansion_factor() returns the value of this multiplier.  To be
                  very specific, it is always true that M==I*N where I is some integer.
                  This integer I is what is returned by sample_expansion_factor().

                  It should be noted that computational layers likely do not care about the
                  sample expansion factor.  It is only really of concern inside a loss
                  layer where you need to know its value so that tensor samples can be
                  matched against truth objects.  Moreover, in most cases the sample
                  expansion factor is 1.
        !*/

    };

// ----------------------------------------------------------------------------------------

    class EXAMPLE_COMPUTATIONAL_LAYER_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                Each computational layer in a deep neural network can be thought of as a
                function, f(data,parameters), that takes in a data tensor, some parameters,
                and produces an output tensor.  You create an entire deep network by
                composing these functions.  Importantly, you are able to use a wide range
                of different functions to accommodate the task you are trying to
                accomplish.  Therefore, dlib includes a number of common layer types but if
                you want to define your own then you simply implement a class with the same
                interface as EXAMPLE_COMPUTATIONAL_LAYER_.

                Note that there is no dlib::EXAMPLE_COMPUTATIONAL_LAYER_ type.  It is shown
                here purely to document the interface that a layer object must implement.

                The central work of defining a layer is implementing the forward and backward
                methods.  When you do this you have four options:
                    - Implement the forward() and backward() methods according to the
                      specification shown below.  Do not implement forward_inplace() and
                      backward_inplace().
                    - Implement the forward() and backward() methods according to the
                      specification shown below, except exclude the computed_output
                      parameter from backward().  Doing this will allow dlib to make some
                      layers execute in-place and therefore run a little faster and use
                      less memory. Do not implement forward_inplace() and
                      backward_inplace().
                    - Implement the forward_inplace() and backward_inplace() methods
                      according to the specification shown below.  Do not implement
                      forward() and backward().  These in-place methods allow some types of
                      layers to be implemented more efficiently.
                    - Implement the forward_inplace() and backward_inplace() methods
                      according to the specification shown below, except exclude the
                      computed_output parameter from backward_inplace().  Doing this will
                      allow dlib to make some layers execute in-place and therefore run a
                      little faster and use less memory.  Do not implement forward() and
                      backward().


                It should also be noted that layers may define additional layer specific
                fields and the solvers can use these fields as they see fit.  For example,
                some layers define get_learning_rate_multiplier() and
                get_weight_decay_multiplier() methods.  The solvers that come with dlib
                look at these methods, if they exist, and adjust the learning rate or
                weight decay for that layer according to the multiplier.  Therefore, you
                can add these methods to your layer types if you want, or even define new
                fields and new solvers that use those fields in some way.  
        !*/

    public:

        EXAMPLE_COMPUTATIONAL_LAYER_(
        );
        /*!
            ensures
                - Default constructs this object.  This function is not required to do
                  anything in particular but it must exist, that is, it is required that
                  layer objects be default constructable. 
        !*/

        EXAMPLE_COMPUTATIONAL_LAYER_ (
            const EXAMPLE_COMPUTATIONAL_LAYER_& item
        );
        /*!
            ensures
                - EXAMPLE_COMPUTATIONAL_LAYER_ objects are copy constructable
        !*/

        EXAMPLE_COMPUTATIONAL_LAYER_(
            const some_other_layer_type& item
        );
        /*!
            ensures
                - Constructs this object from item.  This form of constructor is optional
                  but it allows you to provide a conversion from one layer type to another.
                  For example, the following code is valid only if my_layer2 can be
                  constructed from my_layer1:
                    relu<fc<my_layer1<fc<input<matrix<float>>>>>> my_dnn1;
                    relu<fc<my_layer2<fc<input<matrix<float>>>>>> my_dnn2(my_dnn1);
                  This kind of pattern is useful if you want to use one type of layer
                  during training but a different type of layer during testing since it
                  allows you to easily convert between related deep neural network types.  

                  Additionally, if you provide a constructor to build a layer from another
                  layer type you should also write your layer's deserialize() routine such
                  that it can read that other layer's serialized data in addition to your
                  own serialized data.  
        !*/

        template <typename SUBNET>
        void setup (
            const SUBNET& sub
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
            ensures
                - performs any necessary initial memory allocations and/or sets parameters
                  to their initial values prior to learning.  Therefore, calling setup
                  destroys any previously learned parameters.  Also, typically setup()
                  would look at the dimensions of the outputs of sub and configure the
                  number of parameters in *this accordingly.
        !*/

        template <typename SUBNET>
        void forward(
            const SUBNET& sub, 
            resizable_tensor& data_output
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
            ensures
                - Runs the output of the subnetwork through this layer and stores the
                  results into #data_output.  In particular, forward() can use any of the
                  outputs in sub (e.g. sub.get_output(), sub.subnet().get_output(), etc.)
                  to compute whatever it wants.
        !*/

        template <typename SUBNET>
        void backward(
            const tensor& computed_output, // this parameter is optional
            const tensor& gradient_input, 
            SUBNET& sub, 
            tensor& params_grad
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
                - computed_output is the tensor resulting from calling forward(sub,computed_output).  
                  Moreover, this was the most recent call to forward().  This means that
                  forward() is allowed to cache intermediate results so they can be used
                  during the backward computation.
                - have_same_dimensions(gradient_input, computed_output) == true
                - have_same_dimensions(sub.get_gradient_input(), sub.get_output()) == true
                - have_same_dimensions(params_grad, get_layer_params()) == true
            ensures
                - This function outputs the gradients of this layer with respect to the
                  input data from sub and also with respect to this layer's parameters.
                  These gradients are stored into #sub and #params_grad, respectively. To be
                  precise, the gradients are taken of a function f(sub,get_layer_params())
                  which is defined thusly:   
                    - Recalling that computed_output is a function of both sub and get_layer_params(), 
                      since it is the result of calling forward(sub,computed_output):
                      let f(sub,get_layer_params()) == dot(computed_output, gradient_input)
                  Then we define the following gradient vectors: 
                    - PARAMETER_GRADIENT == gradient of f(sub,get_layer_params()) with
                      respect to get_layer_params(). 
                    - for all valid I:
                        - DATA_GRADIENT_I == gradient of f(sub,get_layer_params()) with
                          respect to layer<I>(sub).get_output() (recall that forward() can
                          draw inputs from the immediate sub layer, sub.subnet(), or
                          any earlier layer.  So you must consider the gradients with
                          respect to all inputs drawn from sub)
                  Finally, backward() outputs these gradients by performing:
                    - params_grad = PARAMETER_GRADIENT 
                    - for all valid I:
                        - layer<I>(sub).get_gradient_input() += DATA_GRADIENT_I
        !*/

        void forward_inplace(
            const tensor& data_input, 
            tensor& data_output
        );
        /*!
            requires
                - have_same_dimensions(data_input,data_output) == true
                - setup() has been called.
            ensures
                - Runs the data_input tensor through this layer and stores the output into
                  #data_output.
                - This function supports in-place operation, i.e. having
                  is_same_object(data_input, data_output)==true
        !*/

        void backward_inplace(
            const tensor& computed_output, // this parameter is optional
            const tensor& gradient_input,
            tensor& data_grad,
            tensor& params_grad
        );
        /*!
            requires
                - setup() has been called.
                - computed_output is the tensor resulting from the most recent call to
                  forward_inplace().  This means that forward_inplace() is allowed to cache
                  intermediate results so they can be used during the backward computation.
                - have_same_dimensions(gradient_input, data_grad) == true
                - have_same_dimensions(gradient_input, computed_output) == true
                - have_same_dimensions(params_grad, get_layer_params()) == true
            ensures
                - This function supports in-place operation, i.e. having
                  is_same_object(gradient_input, data_grad)==true
                - This function outputs the gradients of this layer with respect to the
                  input data from a sublayer and also with respect to this layer's parameters.
                  These gradients are stored into #data_grad and #params_grad, respectively. To be
                  precise, the gradients are taken of a function f(data_input,get_layer_params())
                  which is defined thusly:   
                    - Recalling that computed_output is a function of both the input to
                      forward_inplace() and get_layer_params(), since it is the result of
                      calling forward_inplace(data_input,computed_output):
                      let f(data_input,get_layer_params()) == dot(computed_output, gradient_input)
                  Then we define the following gradient vectors: 
                    - PARAMETER_GRADIENT == gradient of f(data_input,get_layer_params()) with
                      respect to get_layer_params(). 
                    - DATA_GRADIENT == gradient of f(data_input,get_layer_params()) with respect
                      to data_input. 
                  Finally, backward_inplace() outputs these gradients by performing:
                    - params_grad = PARAMETER_GRADIENT 
                    - if (is_same_object(gradient_input, data_grad)) then
                        - data_grad = DATA_GRADIENT
                    - else
                        - data_grad += DATA_GRADIENT
        !*/

        const tensor& get_layer_params(
        ) const; 
        /*!
            ensures
                - returns the parameters that define the behavior of forward().
        !*/

        tensor& get_layer_params(
        ); 
        /*!
            ensures
                - returns the parameters that define the behavior of forward().
        !*/


        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        /*!
            These two functions are optional.  If provided, they should map between
            (column,row) coordinates in input and output tensors of forward().  Providing
            these functions allows you to use global utility functions like
            input_tensor_to_output_tensor().
        !*/

        void clean (
        );
        /*!
            Implementing this function is optional.  If you don't need it then you don't
            have to provide a clean().  But if you do provide it then it must behave as
            follows:

            ensures
                - calling clean() causes this object to forget about everything except its
                  parameters.  This is useful if your layer caches information between
                  forward and backward passes and you want to clean out that cache
                  information before saving the network to disk.  
        !*/

    };

    std::ostream& operator<<(std::ostream& out, const EXAMPLE_COMPUTATIONAL_LAYER_& item);
    /*!
        print a string describing this layer.
    !*/

    void to_xml(const EXAMPLE_COMPUTATIONAL_LAYER_& item, std::ostream& out);
    /*!
        This function is optional, but required if you want to print your networks with
        net_to_xml().  Therefore, to_xml() prints a layer as XML.
    !*/

    void serialize(const EXAMPLE_COMPUTATIONAL_LAYER_& item, std::ostream& out);
    void deserialize(EXAMPLE_COMPUTATIONAL_LAYER_& item, std::istream& in);
    /*!
        provides serialization support  
    !*/

    // For each layer you define, always define an add_layer template so that layers can be
    // easily composed.  Moreover, the convention is that the layer class ends with an _
    // while the add_layer template has the same name but without the trailing _.
    template <typename SUBNET>
    using EXAMPLE_COMPUTATIONAL_LAYER = add_layer<EXAMPLE_COMPUTATIONAL_LAYER_, SUBNET>;

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    enum fc_bias_mode
    {
        FC_HAS_BIAS = 0,
        FC_NO_BIAS = 1
    };

    struct num_fc_outputs
    {
        num_fc_outputs(unsigned long n) : num_outputs(n) {}
        unsigned long num_outputs;
    };

    template <
        unsigned long num_outputs,
        fc_bias_mode bias_mode
        >
    class fc_
    {
        /*!
            REQUIREMENTS ON num_outputs
                num_outputs > 0

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a fully connected layer that
                takes an input tensor and multiplies it by a weight matrix and outputs the
                results.

                The dimensions of the tensors output by this layer are as follows (letting
                IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == get_num_outputs()
                    - OUT.nr() == 1
                    - OUT.nc() == 1
        !*/

    public:

        fc_(
        );
        /*!
            ensures
                - #get_num_outputs() == num_outputs
                - #get_bias_mode() == bias_mode 
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        fc_(
            num_fc_outputs o
        );
        /*!
            ensures
                - #get_num_outputs() == o.num_outputs 
                - #get_bias_mode() == bias_mode 
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        unsigned long get_num_outputs (
        ) const; 
        /*!
            ensures
                - This layer outputs column vectors that contain get_num_outputs()
                  elements. That is, the output tensor T from forward() will be such that:
                    - T.num_samples() == however many samples were given to forward().
                    - T.k() == get_num_outputs()
                    - The rest of the dimensions of T will be 1.
        !*/

        void set_num_outputs(
            long num
        );
        /*!
            requires
                - num > 0
                - get_layer_params().size() == 0 || get_num_outputs() == num
                  (i.e. You can't change the number of outputs in fc_ if the parameter
                  tensor has already been allocated.)
            ensures
                - #get_num_outputs() == num
        !*/

        fc_bias_mode get_bias_mode (
        ) const;
        /*!
            ensures
                - returns the bias mode which determines if this layer includes bias terms.
                  That is, if the bias mode is FC_HAS_BIAS then a different constant scalar
                  is added to each of the outputs of this layer. 
        !*/

        double get_learning_rate_multiplier(
        ) const;  
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its parameters be
                  multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its parameters be
                  multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its bias parameters be
                  multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its bias parameters be
                  multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        void disable_bias(
        );
        /*!
            ensures
                - bias_is_disabled() returns true
        !*/

        bool bias_is_disabled(
        ) const;
        /*!
            ensures
                - returns true if bias learning is disabled for this layer.  This means the biases will
                  not be learned during the training and they will not be used in the forward or backward
                  methods either.
        !*/

        alias_tensor_const_instance get_weights(
        ) const;
        /*!
            ensures
                - returns an alias of get_layer_params(), containing the weights matrix of
                  the fully connected layer.
                - #get_weights().num_samples() is the number of elements in input sample,
                  i.e. sublayer's output's k * nc * nr.
                - #get_bias().k() == #get_num_outputs()
                - if get_bias_mode() == FC_HAS_BIAS:
                    - #get_layer_params().size() == (#get_weights().size() + #get_biases().size())
                - else:
                    - #get_layer_params().size() == #get_weights().size()
        !*/

        alias_tensor_instance get_weights(
        );
        /*!
            ensures
                - returns an alias of get_layer_params(), containing the weights matrix of
                  the fully connected layer.
                - #get_weights().num_samples() is the number of elements in input sample,
                  i.e. sublayer's output's k * nc * nr.
                - #get_bias().k() == #get_num_outputs()
                - if get_bias_mode() == FC_HAS_BIAS:
                    - #get_layer_params().size() == (#get_weights().size() + #get_biases().size())
                - else:
                    - #get_layer_params().size() == #get_weights().size()
        !*/

        alias_tensor_const_instance get_biases(
        ) const;
        /*!
            requires
                - #get_bias_mode() == FC_HAS_BIAS
            ensures
                - returns an alias of get_layer_params(), containing the bias vector of
                  the fully connected layer.
                - #get_bias().num_samples() == 1
                - #get_bias().k() == #get_num_outputs()
                - #get_layer_params().size() == (#get_weights().size() + #get_biases().size())
        !*/

        alias_tensor_instance get_biases(
        );
        /*!
            requires
                - #get_bias_mode() == FC_HAS_BIAS
            ensures
                - returns an alias of get_layer_params(), containing the bias vector of
                  the fully connected layer.
                - #get_bias().num_samples() == 1
                - #get_bias().k() == #get_num_outputs()
                - #get_layer_params().size() == (#get_weights().size() + #get_biases().size())
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/

    };

    template <
        unsigned long num_outputs,
        typename SUBNET
        >
    using fc = add_layer<fc_<num_outputs,FC_HAS_BIAS>, SUBNET>;

    template <
        unsigned long num_outputs,
        typename SUBNET
        >
    using fc_no_bias = add_layer<fc_<num_outputs,FC_NO_BIAS>, SUBNET>;

    // ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------

    enum linear_bias_mode
    {
        LINEAR_HAS_BIAS,
        LINEAR_NO_BIAS
    };

    template <
        unsigned long num_outputs,
        linear_bias_mode bias_mode = LINEAR_HAS_BIAS
    >
    class linear_
    {
        /*!
            REQUIREMENTS ON num_outputs
                num_outputs > 0

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of a linear layer, which applies a linear
                transformation to the input data. For a layer with bias, the transformation
                is:
                    output = input * weights + bias
                For a layer without bias, it's simply:
                    output = input * weights

                The input tensor can have any number of sample, k (channel), and nr (row)
                dimensions, but the nc (column) dimension must match the number of input features.
                The output tensor will have the same dimensions as the input tensor, except for
                the nc dimension which will be equal to num_outputs.

                This layer is similar to the fc_ layer, but optimized for the case where the
                input and output tensors maintain the same dimensions, excluding the feature
                dimension (nc). This makes it useful for working with multi-dimensional data.
        !*/

    public:
        linear_(
        );
        /*!
            ensures
                - #get_num_outputs() == num_outputs
                - #get_bias_mode() == bias_mode
                - #get_learning_rate_multiplier() == 1
        !*/

        double get_learning_rate_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier that will be applied to the gradient of this layer during
                  training. This value appears as a multiplicative factor in the update rule. So
                  if get_learning_rate_multiplier() == 1 then the learning rate will be multiplied
                  by 1 and thus not modified. However, if get_learning_rate_multiplier() == 0.1 then
                  the learning rate will be multiplied by 0.1, making the layer update 10 times
                  slower than it would otherwise be.
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        unsigned long get_num_inputs(
        ) const;
        /*!
            ensures
                - Returns the number of input features this layer expects.
                - For an uninitialized layer (i.e., one that has not seen any data during setup
                  or forward pass), this will be zero.
        !*/

        unsigned long get_num_outputs(
        ) const;
        /*!
            ensures
                - Returns the number of output features this layer produces.
                  I.e., this value is num_outputs.
        !*/

        void set_num_outputs(
            long num
        );
        /*!
            requires
                - num > 0
            ensures
                - #get_num_outputs() == num
            throws
                - std::runtime_error if this function is called after the layer parameters
                  have been allocated and the new number of outputs doesn't match the
                  previously set number of outputs.
        !*/

        linear_bias_mode get_bias_mode(
        ) const;
        /*!
            ensures
                - Returns a value indicating whether this layer has a bias term.
                  I.e. returns bias_mode.
        !*/

        template <typename SUBNET>
        void setup(
            const SUBNET& sub
        );
        /*!
            ensures
                - Performs the necessary setup work to process data through this layer.
                - Sets the input size based on the dimensions of the input tensor from sub.
                - Allocates the parameter tensor and initializes its values.
                - #get_num_inputs() == the number of columns in sub.get_output() (i.e., nc).
        !*/

        template <typename SUBNET>
        void forward(
            const SUBNET& sub,
            resizable_tensor& output
        );
        /*!
            requires
                - setup() has been called
                - sub.get_output().nc() == get_num_inputs()
            ensures
                - Applies the linear transformation to the input tensor from sub and stores
                  the results in output.
                - #output.num_samples() == sub.get_output().num_samples()
                - #output.k()           == sub.get_output().k()
                - #output.nr()          == sub.get_output().nr()
                - #output.nc()          == get_num_outputs()
        !*/

        template <typename SUBNET>
        void backward(
            const tensor& gradient_input,
            SUBNET& sub,
            tensor& params_grad
        );
        /*!
            requires
                - setup() has been called
                - sub.get_output().nc() == get_num_inputs()
                - gradient_input has the same dimensions as the output of forward()
            ensures
                - Computes the gradients of this layer with respect to the parameters
                  and the input tensor, and updates the corresponding gradient tensors.
                - Updates params_grad based on the gradients of the weights
                  and biases (if present).
                - Updates sub's gradient_input based on the gradients of the
                  inputs to this layer.
        !*/

        alias_tensor_instance get_weights(
        );
        /*!
            requires
                - setup() has been called
            ensures
                - Returns a reference to the weights matrix of this layer.
        !*/

        alias_tensor_const_instance get_weights(
        ) const;
        /*!
            requires
                - setup() has been called
            ensures
                - Returns a const reference to the weights matrix of this layer.
        !*/

        alias_tensor_instance get_biases(
        );
        /*!
            requires
                - bias_mode == LINEAR_HAS_BIAS
                - setup() has been called
            ensures
                - Returns a reference to the bias vector of this layer.
            throws
                - static_assert failure if bias_mode != LINEAR_HAS_BIAS
        !*/

        alias_tensor_const_instance get_biases(
        ) const;
        /*!
            requires
                - bias_mode == LINEAR_HAS_BIAS
                - setup() has been called
            ensures
                - Returns a const reference to the bias vector of this layer.
            throws
                - static_assert failure if bias_mode != LINEAR_HAS_BIAS
        !*/

        dpoint map_input_to_output(
            const dpoint& p
        ) const;
        /*!
            ensures
                - Returns p, since the linear layer maintains the same spatial dimensions.
        !*/

        dpoint map_output_to_input(
            const dpoint& p
        ) const;
        /*!
            ensures
                - Returns p, since the linear layer maintains the same spatial dimensions.
        !*/

        const tensor& get_layer_params(
        ) const;
        /*!
            ensures
                - Returns the parameters that define this layer, i.e., the weights and biases
                  (if present) that are updated during training.
        !*/

        tensor& get_layer_params(
        );
        /*!
            ensures
                - Returns the parameters that define this layer, i.e., the weights and biases
                  (if present) that are updated during training.
        !*/

        friend void serialize(const linear_& item, std::ostream& out);
        friend void deserialize(linear_& item, std::istream& in);
        /*!
            provides serialization support
        !*/
    };

    template <
        unsigned long num_outputs,
        typename SUBNET
    >
    using linear = add_layer<linear_<num_outputs, LINEAR_HAS_BIAS>, SUBNET>;
    /*!
        This is a layer that applies a linear transformation with bias to the input:
        output = input * weights + bias
    !*/

    template <
        unsigned long num_outputs,
        typename SUBNET
    >
    using linear_no_bias = add_layer<linear_<num_outputs, LINEAR_NO_BIAS>, SUBNET>;
    /*!
        This is a layer that applies a linear transformation without bias to the input:
        output = input * weights
    !*/

    // ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------

    struct num_con_outputs
    {
        num_con_outputs(unsigned long n) : num_outputs(n) {}
        unsigned long num_outputs;
    };

    template <
        long _num_filters,
        long _nr,
        long _nc,
        int _stride_y,
        int _stride_x,
        int _padding_y = _stride_y!=1? 0 : _nr/2,
        int _padding_x = _stride_x!=1? 0 : _nc/2
        >
    class con_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - _num_filters > 0
                - _nr >= 0
                - _nc >= 0
                - _stride_y > 0
                - _stride_x > 0
                - _padding_y >= 0
                - _padding_x >= 0
                - Also, we require that:
                    - if (_nr == 0) then
                        - _padding_y == 0
                    - else
                        - _padding_y < _nr
                    - if (_nc == 0) then
                        - _padding_x == 0
                    - else
                        - _padding_x < _nc

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a convolution layer that takes an
                input tensor (nominally representing an image) and convolves it with a set
                of filters and then outputs the results. 

                The dimensions of the tensors output by this layer are as follows (letting
                IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == num_filters()
                    - OUT.nr() == 1+(IN.nr() + 2*padding_y() - nr())/stride_y()
                    - OUT.nc() == 1+(IN.nc() + 2*padding_x() - nc())/stride_x()

                Note also that setting _nr or _nc to 0 has a special meaning of "set the
                filter size equal to the input image size".  Specifically, it means: 
                    - if (_nr == 0) then
                        - nr() == IN.nr()
                        - OUT.nr() == 1
                    - if (_nc == 0) then
                        - nc() == IN.nc()
                        - OUT.nc() == 1
        !*/

    public:
        con_(
        );
        /*!
            ensures
                - #num_filters() == _num_filters
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        con_(
            num_con_outputs o
        );
        /*!
            ensures
                - #num_filters() == o.num_outputs 
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        long num_filters(
        ) const; 
        /*!
            ensures
                - returns the number of filters contained in this layer.  The k dimension
                  of the output tensors produced by this layer will be equal to the number
                  of filters.
        !*/

        void set_num_filters(
            long num
        );
        /*!
            requires
                - num > 0
                - get_layer_params().size() == 0 || num_filters() == num
                  (i.e. You can't change the number of filters in con_ if the parameter
                  tensor has already been allocated.)
            ensures
                - #num_filters() == num
        !*/

        long nr(
        ) const; 
        /*!
            ensures
                - returns the number of rows in the filters in this layer.  Note that if
                  nr()==0 then it means the size of the filter is not yet assigned, but
                  once setup() is called nr() will be set to the input tensor's nr().
                  Therefore, nr()==0 has the special interpretation of "be the same size as
                  the input tensor".
        !*/

        long nc(
        ) const;
        /*!
            ensures
                - returns the number of columns in the filters in this layer.  Note that if
                  nc()==0 then it means the size of the filter is not yet assigned, but
                  once setup() is called nc() will be set to the input tensor's nc().
                  Therefore, nc()==0 has the special interpretation of "be the same size as
                  the input tensor".
        !*/

        long stride_y(
        ) const; 
        /*!
            ensures
                - returns the vertical stride used when convolving the filters over an
                  image.  That is, each filter will be moved stride_y() pixels down at a
                  time when it moves over the image.
        !*/

        long stride_x(
        ) const;
        /*!
            ensures
                - returns the horizontal stride used when convolving the filters over an
                  image.  That is, each filter will be moved stride_x() pixels right at a
                  time when it moves over the image.
        !*/

        long padding_y(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the top and bottom
                  sides of the image.
        !*/

        long padding_x(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the left and right 
                  sides of the image.
        !*/

        double get_learning_rate_multiplier(
        ) const;  
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its parameters be
                  multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its parameters be
                  multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its bias parameters be
                  multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its bias parameters be
                  multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        void disable_relu(
        );
        /*!
            ensures
                - relu_is_disabled() returns true
        !*/

        void enable_relu(
        );
        /*!
            ensures
                - relu_is_disabled() returns false
        !*/

        bool relu_is_disabled(
        ) const;
        /*!
            ensures
                - returns true if relu is disabled for this layer. This means no activation function
                  will be applied after the convolution when calling forward.
        !*/

        void disable_bias(
        );
        /*!
            ensures
                - bias_is_disabled() returns true
                - if bias was enabled and allocated, it resizes the layer parameters
                  to accommodate the filter parameters only, and free the bias parameters.
        !*/

        void enable_bias(
        );
        /*!
            ensures
                - bias_is_disabled() returns false
                - if bias was disabled and not allocated, it resizes the layer parameters
                  to accommodate the new zero-inizialized biases
        !*/

        bool bias_is_disabled(
        ) const;
        /*!
            ensures
                - returns true if bias learning is disabled for this layer.  This means the biases will
                  not be learned during the training and they will not be used in the forward or backward
                  methods either.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/

    };

    template <
        long num_filters,
        long nr,
        long nc,
        int stride_y,
        int stride_x,
        typename SUBNET
        >
    using con = add_layer<con_<num_filters,nr,nc,stride_y,stride_x>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        long _num_filters,
        long _nr,
        long _nc,
        int _stride_y,
        int _stride_x,
        int _padding_y = _stride_y!=1? 0 : _nr/2,
        int _padding_x = _stride_x!=1? 0 : _nc/2
        >
    class cont_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                All of them must be > 0.
                Also, we require that:
                    - 0 <= _padding_y && _padding_y < _nr
                    - 0 <= _padding_x && _padding_x < _nc

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a transposed convolution layer
                that takes an input tensor and transpose convolves (sometimes called
                "deconvolution") it with a set of filters and then outputs the results. 

                This is essentially a convolutional layer that allows fractional strides.
                Therefore, you can make output tensors that are larger than the input
                tensors using this layer type. 

                
                The dimensions of the tensors output by this layer are as follows (letting
                IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == num_filters()
                    - OUT.nr() == stride_y()*(IN.nr()-1) + nr() - 2*padding_y()
                    - OUT.nc() == stride_x()*(IN.nc()-1) + nc() - 2*padding_x()
        !*/

    public:
        cont_(
        );
        /*!
            ensures
                - #num_filters() == _num_filters
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        cont_(
            num_con_outputs o
        );
        /*!
            ensures
                - #num_filters() == o.num_outputs 
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 1
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 0
        !*/

        long num_filters(
        ) const; 
        /*!
            ensures
                - returns the number of filters contained in this layer.  The k dimension
                  of the output tensors produced by this layer will be equal to the number
                  of filters.
        !*/

        void set_num_filters(
            long num
        );
        /*!
            requires
                - num > 0
                - get_layer_params().size() == 0 || num_filters() == num
                  (i.e. You can't change the number of filters in cont_ if the parameter
                  tensor has already been allocated.)
            ensures
                - #num_filters() == num
        !*/

        long nr(
        ) const; 
        /*!
            ensures
                - returns the number of rows in the filters in this layer.
        !*/

        long nc(
        ) const;
        /*!
            ensures
                - returns the number of columns in the filters in this layer.
        !*/

        long stride_y(
        ) const; 
        /*!
            ensures
                - returns the vertical stride used when convolving the filters over an
                  image.  That is, each filter will be moved 1.0/stride_y() pixels down at
                  a time when it moves over the image.
        !*/

        long stride_x(
        ) const;
        /*!
            ensures
                - returns the horizontal stride used when convolving the filters over an
                  image.  That is, each filter will be moved 1.0/stride_x() pixels right at
                  a time when it moves over the image.
        !*/

        long padding_y(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the top and bottom
                  sides of the image.
        !*/

        long padding_x(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the left and right 
                  sides of the image.
        !*/

        double get_learning_rate_multiplier(
        ) const;  
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its parameters be
                  multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its parameters be
                  multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its bias parameters be
                  multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its bias parameters be
                  multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        void disable_bias(
        );
        /*!
            ensures
                - bias_is_disabled() returns true
        !*/

        bool bias_is_disabled(
        ) const;
        /*!
            ensures
                - returns true if bias learning is disabled for this layer.  This means the biases will
                  not be learned during the training and they will not be used in the forward or backward
                  methods either.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/

    };

    template <
        long num_filters,
        long nr,
        long nc,
        int stride_y,
        int stride_x,
        typename SUBNET
        >
    using cont = add_layer<cont_<num_filters,nr,nc,stride_y,stride_x>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        int scale_y, 
        int scale_x 
        >
    class upsample_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                All of them must be >= 1.

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it allows you to upsample a layer using
                bilinear interpolation.  To be very specific, it upsamples each of the
                channels in an input tensor.  Therefore, if IN is the input tensor to this
                layer and OUT the output tensor, then we will have:
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k() 
                    - OUT.nr() == IN.nr()*scale_y
                    - OUT.nc() == IN.nc()*scale_x
                    - for all valid i,k:  image_plane(OUT,i,k) is a copy of
                      image_plane(IN,i,k) that has been bilinearly interpolated to fit into
                      the shape of image_plane(OUT,i,k).
        !*/
    public:

        upsample_(
        );
        /*!
            ensures
                - This object has no state, so the constructor does nothing, aside from
                  providing default constructability.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        int scale,
        typename SUBNET
        >
    using upsample = add_layer<upsample_<scale,scale>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        long NR_, 
        long NC_
        >
    class resize_to_
    {
        /*!
            REQUIREMENTS ON THE INPUT ARGUMENTS
                - NR_ >= 1
                - NC_ >= 1

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it allows you to resize a layer using
                bilinear interpolation.  To be very specific, it resizes each of the
                channels in an input tensor.  Therefore, if IN is the input tensor to this
                layer and OUT the output tensor, then we will have:
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k() 
                    - OUT.nr() == NR_
                    - OUT.nc() == NC_
                    - for all valid i,k:  image_plane(OUT,i,k) is a copy of
                      image_plane(IN,i,k) that has been bilinearly interpolated to fit into
                      the shape of image_plane(OUT,i,k).
        !*/
    public:

        resize_to_(
        );
        /*!
            ensures
                - This object has no state, so the constructor does nothing, aside from
                  providing default constructability.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        long NR,
        long NC,
        typename SUBNET
        >
    using resize_to = add_layer<resize_to_<NR,NC>, SUBNET>;
    
// ----------------------------------------------------------------------------------------

    template <long k_ = -1, long nr_ = -1, long nc_ = -1>
    class reshape_to_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - k_, nr_, and nc_ must be either -1 or greater than 0.

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above. It defines a layer that reshapes or resizes an input tensor
                into a different shape. The layer operates in two modes:

                1. Pure Reshape Mode: When the total number of elements in the input tensor
                   equals the total number of elements in the output tensor, this layer
                   performs a simple reshaping operation without changing the values.

                2. Spatial Rescaling Mode: When the channel dimension (k) remains constant
                   but the total number of elements changes, this layer performs bilinear
                   interpolation to resize the spatial dimensions while preserving the
                   channel information.

                The dimensions of the output tensor are determined by the template parameters:
                    - If k_ is -1, the output tensor will have the same number of channels as the input.
                    - If nr_ is -1, the output tensor will have the same number of rows as the input.
                    - If nc_ is -1, the output tensor will have the same number of columns as the input.

                Setting a value of -1 for any dimension means "keep the original dimension from the input."

                Note that this layer will throw an exception if you attempt to change both the
                channel count (k) and the total number of elements. Either:
                - Keep the total number of elements the same (Pure Reshape Mode), or
                - Keep the channel count the same and only change spatial dimensions (Spatial Rescaling Mode)
        !*/

    public:
        explicit reshape_to_();
        /*!
            ensures
                - #get_output_k() == k_
                - #get_output_nr() == nr_
                - #get_output_nc() == nc_
        !*/

        long get_output_k() const;
        /*!
            ensures
                - Returns the number of channels in the output tensor. If this value is -1,
                  then the output will have the same number of channels as the input.
        !*/

        long get_output_nr() const;
        /*!
            ensures
                - Returns the number of rows in the output tensor. If this value is -1,
                  then the output will have the same number of rows as the input.
        !*/

        long get_output_nc() const;
        /*!
            ensures
                - Returns the number of columns in the output tensor. If this value is -1,
                  then the output will have the same number of columns as the input.
        !*/

        void set_output_k(long k);
        /*!
            requires
                - k == -1 || k > 0
            ensures
                - #get_output_k() == k
        !*/

        void set_output_nr(long nr);
        /*!
            requires
                - nr == -1 || nr > 0
            ensures
                - #get_output_nr() == nr
        !*/

        void set_output_nc(long nc);
        /*!
            requires
                - nc == -1 || nc > 0
            ensures
                - #get_output_nc() == nc
        !*/

        template <typename SUBNET> void setup(const SUBNET& sub);
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
            ensures
                - Configures this layer to operate on the output of sub.
                - If the total number of elements in the input tensor doesn't match the total
                  number of elements in the output tensor and the channel dimension is different,
                  an exception will be thrown.
        !*/

        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
            ensures
                - Reshapes or resizes the output of sub and stores it in #output.
                - If is_spatial_rescale() == false, then performs a pure reshape operation.
                - If is_spatial_rescale() == true, then performs bilinear interpolation to resize
                  the spatial dimensions while preserving the channel information.
                - #output.num_samples() == sub.get_output().num_samples()
                - #output.k() == get_output_k() if get_output_k() != -1, otherwise sub.get_output().k()
                - #output.nr() == get_output_nr() if get_output_nr() != -1, otherwise sub.get_output().nr()
                - #output.nc() == get_output_nc() if get_output_nc() != -1, otherwise sub.get_output().nc()
        !*/

        template <typename SUBNET> void backward(
            const tensor& gradient_input,
            SUBNET& sub,
            tensor& params_grad
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
                - gradient_input has the same dimensions as the output of forward().
            ensures
                - Computes the gradients of this layer with respect to the input tensor and
                  parameters, and stores them in sub.get_gradient_input() and params_grad,
                  respectively.
                - This function supports both pure reshaping and spatial rescaling operations.
        !*/

        dpoint map_input_to_output(dpoint p) const;
        /*!
            ensures
                - Maps a point in the input tensor's coordinate system to the corresponding point
                  in the output tensor. This is useful for tracking how spatial locations change
                  through the network, especially during spatial rescaling.
        !*/

        dpoint map_output_to_input(dpoint p) const;
        /*!
            ensures
                - Maps a point in the output tensor's coordinate system to the corresponding point
                  in the input tensor. This is the inverse of map_input_to_output().
        !*/

        const tensor& get_layer_params() const;
        /*!
            ensures
                - Returns the layer's parameters. This layer has no parameters,
                  so this always returns an empty tensor.
        !*/

        tensor& get_layer_params();
        /*!
            ensures
                - Returns the layer's parameters. This layer has no parameters,
                  so this always returns an empty tensor.
        !*/
    };

    template <long k, long nr, long nc, typename SUBNET>
    using reshape_to = add_layer<reshape_to_<k, nr, nc>, SUBNET>;

    template <long k, long nr, long nc, typename SUBNET>
    using flatten = add_layer<reshape_to_<k * nr, * nc, 1, 1>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class dropout_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a dropout layer.  Therefore, it
                passes its inputs through the stochastic function f(x) which outputs either
                0 or x.  The probability of 0 being output is given by the drop_rate
                argument to this object's constructor.

                Note that, after you finish training a network with dropout, it is a good
                idea to replace each dropout_ layer with a multiply_ layer because the
                multiply_ layer is faster and deterministic. 
        !*/

    public:

        explicit dropout_(
            float drop_rate = 0.5
        );
        /*!
            requires
                - 0 <= drop_rate <= 1
            ensures
                - #get_drop_rate() == drop_rate
        !*/

        float get_drop_rate (
        ) const; 
        /*!
            ensures
                - returns the probability that an individual input value to this layer will
                  be replaced with 0.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using dropout = add_layer<dropout_, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <int DROP_RATE_PERCENT>
    class dropout_rate_ : public dropout_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object represents a customizable dropout layer that inherits from
                the dropout_ class. It allows specifying the dropout rate at compile-time,
                which is particularly useful for deep networks with many layers where it
                might be cumbersome to explicitly modify the dropout rate for each layer
                individually.

                The main advantage of this layer is that it offers the possibility to specify
                the dropout rate at the moment of network construction, providing more
                flexibility and clarity in the network architecture definition.

            TEMPLATE PARAMETERS
                - DROP_RATE_PERCENT: A int value between 0 and 100 that specifies the dropout rate.
                  This value is set at compile-time and cannot be changed during runtime.
        !*/

    public:
        explicit dropout_rate_();
        /*!
            ensures
                - Constructs a dropout layer with a dropout rate of DROP_RATE.
                - Calls the base class constructor dropout_(DROP_RATE).
        !*/
    };

    template <int DROP_RATE, typename SUBNET>
    using dropout_rate = add_layer<dropout_rate_<DROP_RATE>, SUBNET>;
    template <typename SUBNET>
    using dropout_10 = add_layer<dropout_rate_<10>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class multiply_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a basic layer that just
                multiplies its input tensor with a constant value and returns the result.
                It therefore has no learnable parameters.
        !*/

    public:
        explicit multiply_(
            float val = 0.5
        ); 
        /*!
            ensures
                - #get_multiply_value() == val
        !*/

        multiply_ (
            const dropout_& item
        ); 
        /*!
            ensures
                - #get_multiply_value() == 1-item.get_drop_rate()
                  (i.e. We construct the multiply_ layer so that it is essentially a
                  deterministic version of the given dropout_ layer)
        !*/

        float get_multiply_value (
        ) const;
        /*!
            ensures
                - this layer simply multiplies its input tensor by get_multiply_value() and
                  produces the result as output.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using multiply = add_layer<multiply_, SUBNET>;

// ----------------------------------------------------------------------------------------

    const double DEFAULT_LAYER_NORM_EPS = 1e-5;

    class layer_norm_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a batch normalization layer that
                implements the method described in the paper:
                    Layer Normalization by Jimmy Lei Ba, Jamie Ryan Kiros, Geoffrey E. Hinton

                In particular, this layer produces output tensors with the same
                dimensionality as the input tensors, except that the mean and variances of
                the elements in each sample have been standardized to 0 and 1 respectively.
                This is different from batch normalization, since this layer learns one scaling
                factor and one bias for each sample in the batch, independently.  As a result,
                this layer is batch-size independent.
        !*/
    public:
        layer_norm_(
        );
        /*!
            ensures
                - #get_learning_rate_multiplier()       == 1
                - #get_weight_decay_multiplier()        == 0
                - #get_bias_learning_rate_multiplier()  == 1
                - #get_bias_weight_decay_multiplier()   == 1
                - #get_eps() == DEFAULT_LAYER_NORM_EPS
        !*/

        explicit layer_norm_(
            double eps_ = DEFAULT_LAYER_NORM_EPS
        )
        /*!
            requires
                - eps > 0
            ensures
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 0
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 1
                - #get_eps() == eps
        !*/

        double get_eps(
        ) const;
        /*!
            ensures
                - When doing layer normalization, we are dividing by the standard
                  deviation.  This epsilon value returned by this function is added to the
                  variance to prevent the division from dividing by zero.
        !*/

        double get_learning_rate_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its parameters be
                  multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its parameters be
                  multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its bias parameters be
                  multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its bias parameters be
                  multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

// ----------------------------------------------------------------------------------------

    const float DEFAULT_RMS_NORM_EPS = 1e-5f;

    class rms_norm_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above, specifically defining a root mean square (RMS) normalization layer.

                RMS normalization is a technique that normalizes the input tensor based on the
                root mean square (RMS) of its elements. Unlike traditional layer normalization,
                which both centers and scales the data, RMS normalization only scales by the RMS
                value. This makes it computationally more efficient, as it avoids the need to
                compute the mean and subtract it from each element.

                This layer produces output tensors with the same dimensionality as the input tensors.
                Specifically, for an input tensor with shape [num_samples, k, nr, nc], the RMS
                normalization is applied across the [nr, nc] dimensions independently for each
                element in the [k] dimension and for each sample in the [num_samples] dimension.
                The scaling factor (RMS) and the learnable scaling parameter (gamma) are both of
                size [k].

                The key characteristics of this layer are:
                - The RMS of the elements in each sample is standardized to 1.
                - It does not center the data (i.e., it does not subtract the mean).
                - A learnable scaling factor (gamma) is applied after normalization, allowing the
                model to adapt the scaling dynamically.

                This layer is particularly effective in various natural language processing tasks,
                where it has been shown to provide performance similar to or better than traditional
                layer normalization, with reduced computational overhead.
        !*/

    public:
        rms_norm_(
        );
        /*!
            ensures
                - #get_learning_rate_multiplier() == 1
                - #get_weight_decay_multiplier()  == 0
                - #get_bias_learning_rate_multiplier()  == 1
                - #get_bias_weight_decay_multiplier()   == 1            
                - #get_eps() == DEFAULT_RMS_NORM_EPS
        !*/

        explicit rms_norm_(
            float eps_ = DEFAULT_RMS_NORM_EPS
        );
        /*!
            requires
                - eps > 0
            ensures
                - #get_learning_rate_multiplier() == 1
                - #get_weight_decay_multiplier()  == 0
                - #get_bias_learning_rate_multiplier()  == 1
                - #get_bias_weight_decay_multiplier()   == 1            
                - #get_eps() == eps_
        !*/

        float get_eps(
        ) const;
        /*!
            ensures
                - When doing RMS normalization, we are dividing by the root mean square.
                This epsilon value returned by this function is added to the
                mean square to prevent division by zero.
        !*/

        void set_eps(
            float val
        );
        /*!
            requires
                - val > 0
            ensures
                - #get_eps() == val
        !*/    

        double get_learning_rate_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number. The interpretation is that this object is
                requesting that the learning rate used to optimize its parameters be
                multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number. The interpretation is that this object is
                requesting that the weight decay used to optimize its parameters be
                multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                requesting that the learning rate used to optimize its bias parameters be
                multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const;
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                requesting that the weight decay used to optimize its bias parameters be
                multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using rms_norm = add_layer<rms_norm_, SUBNET>;

// ----------------------------------------------------------------------------------------

    enum layer_mode
    {
        CONV_MODE = 0, // convolutional mode
        FC_MODE = 1    // fully connected mode
    };

    const double DEFAULT_BATCH_NORM_EPS = 0.0001;

    template <
        layer_mode mode
        >
    class bn_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a batch normalization layer that
                implements the method described in the paper: 
                    Batch Normalization: Accelerating Deep Network Training by Reducing
                    Internal Covariate Shift by Sergey Ioffe and Christian Szegedy
                
                In particular, this layer produces output tensors with the same
                dimensionality as the input tensors, except that the mean and variances of
                the elements have been standardized to 0 and 1 respectively. 

                It should also be noted that when tensors with a num_samples() dimension of
                1 are passed to this layer it doesn't perform batch normalization.
                Instead, it runs in "inference mode" where the learned linear normalizing
                transformation is used to transform the tensor. 

                Finally, after you finish training a batch normalized network, it is a good
                idea to replace each bn_ layer with an affine_ layer because the affine_
                layer is faster and will never surprise you by performing batch
                normalization on tensors that have a num_samples() dimension > 1.  This allows
                you to run large mini-batches of samples through your final network without
                batch normalization executing at all. 
        !*/

    public:
        bn_(
        );
        /*!
            ensures
                - #get_mode() == mode
                - #get_running_stats_window_size()      == 100
                - #get_learning_rate_multiplier()       == 1
                - #get_weight_decay_multiplier()        == 0
                - #get_bias_learning_rate_multiplier()  == 1
                - #get_bias_weight_decay_multiplier()   == 1
                - #get_eps() == tt::DEFAULT_BATCH_NORM_EPS
        !*/

        explicit bn_(
            unsigned long window_size,
            double eps = tt::DEFAULT_BATCH_NORM_EPS
        );
        /*!
            requires
                - eps > 0
                - window_size > 0
            ensures
                - #get_mode() == mode 
                - #get_running_stats_window_size()     == window_size
                - #get_learning_rate_multiplier()      == 1
                - #get_weight_decay_multiplier()       == 0
                - #get_bias_learning_rate_multiplier() == 1
                - #get_bias_weight_decay_multiplier()  == 1
                - #get_eps() == eps
        !*/

        layer_mode get_mode(
        ) const; 
        /*!
            ensures
                - returns the mode of this layer, either CONV_MODE or FC_MODE.
                  If the mode is FC_MODE then the normalization is applied across the
                  samples in a tensor (i.e. k()*nr()*nc() different things will be
                  normalized).  Otherwise, normalization is applied across everything
                  except for the k() dimension, resulting in there being only k()
                  normalization equations that are applied spatially over the tensor.

                  Therefore, if you are putting batch normalization after a fully connected
                  layer you should use FC_MODE.  Otherwise, if you are putting batch
                  normalization after a convolutional layer you should use CONV_MODE.
        !*/

        double get_eps(
        ) const; 
        /*!
            ensures
                - When doing batch normalization, we are dividing by the standard
                  deviation.  This epsilon value returned by this function is added to the
                  variance to prevent the division from dividing by zero.
        !*/

        unsigned long get_running_stats_window_size (
        ) const; 
        /*!
            ensures
                - Just as recommended in the batch normalization paper, this object keeps a
                  running average of the mean and standard deviations of the features.
                  These averages are used during "inference mode" so you can run a single
                  object through a batch normalized network.  They are also what is used to
                  initialize an affine_ layer that is constructed from a bn_ layer.  This
                  function returns the effective number of recent samples used to compute
                  the running average.
        !*/

        void set_running_stats_window_size (
            unsigned long new_window_size
        );
        /*!
            requires
                - new_window_size > 0
            ensures
                - #get_running_stats_window_size() == new_window_size
        !*/

        double get_learning_rate_multiplier(
        ) const;  
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its parameters be
                  multiplied by get_learning_rate_multiplier().
        !*/

        double get_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its parameters be
                  multiplied by get_weight_decay_multiplier().
        !*/

        void set_learning_rate_multiplier(
            double val
        );
        /*!
            requires
                - val >= 0
            ensures
                - #get_learning_rate_multiplier() == val
        !*/

        void set_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_weight_decay_multiplier() == val
        !*/

        double get_bias_learning_rate_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the learning rate used to optimize its bias parameters be
                  multiplied by get_learning_rate_multiplier()*get_bias_learning_rate_multiplier().
        !*/

        double get_bias_weight_decay_multiplier(
        ) const; 
        /*!
            ensures
                - returns a multiplier number.  The interpretation is that this object is
                  requesting that the weight decay used to optimize its bias parameters be
                  multiplied by get_weight_decay_multiplier()*get_bias_weight_decay_multiplier().
        !*/

        void set_bias_learning_rate_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_learning_rate_multiplier() == val
        !*/

        void set_bias_weight_decay_multiplier(
            double val
        ); 
        /*!
            requires
                - val >= 0
            ensures
                - #get_bias_weight_decay_multiplier() == val
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using bn_con = add_layer<bn_<CONV_MODE>, SUBNET>;
    template <typename SUBNET>
    using bn_fc = add_layer<bn_<FC_MODE>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class affine_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it applies a simple pointwise linear
                transformation to an input tensor.  You can think of it as having two
                parameter tensors, gamma and beta.  If the input tensor is called INPUT
                then the output of this layer is:
                    gamma*INPUT+beta
                where all operations are performed element wise and each sample in the
                INPUT tensor is processed separately.

                Moreover, this object has two modes that affect the dimensionalities of
                gamma and beta and how they are applied to compute gamma*INPUT+beta.  If
                get_mode()==FC_MODE then gamma and beta each have the same dimensionality
                as the input tensor, except their num_samples() dimensions are 1.  If
                get_mode()==CONV_MODE then gamma and beta have all their dimensions set
                to 1 except for k(), which is equal to INPUT.k().

                In either case, the computation of gamma*INPUT+beta is performed pointwise
                over all the elements of INPUT using either:
                    OUTPUT(n,k,r,c) == gamma(1,k,r,c)*INPUT(n,k,r,c)+beta(1,k,r,c)
                or
                    OUTPUT(n,k,r,c) == gamma(1,k,1,1)*INPUT(n,k,r,c)+beta(1,k,1,1)
                as appropriate.


                Finally, note that the parameters of this layer are not learnable and
                therefore not modified during network updates.  Instead, the layer will
                perform the identity transformation unless it is initialized with a bn_
                layer, in which case it will perform whatever transformation the bn_ layer
                has learned.
        !*/

    public:

        affine_(
        );
        /*!
            ensures
                - #get_mode() == FC_MODE 
        !*/

        affine_(
            layer_mode mode
        );
        /*!
            ensures
                - #get_mode() == mode
        !*/

        template <
            layer_mode mode
            >
        affine_(
            const bn_<mode>& layer
        );
        /*!
            ensures
                - Constructs affine_ so that it performs the same transformation as the
                  supplied batch normalization layer.  You would want to do this after you
                  finish training a network with bn_ layers because the affine_ layer will
                  execute faster.  
                - #get_mode() == layer.get_mode()
        !*/

        layer_mode get_mode(
        ) const; 
        /*!
            ensures
                - returns the mode of this layer, either CONV_MODE or FC_MODE.  
        !*/

        void disable(
        );
        /*!
            ensures
                - #get_layer_params().size() == 0.
                - when forward_inplace and backward_inplace are called, they return immediately doing nothing.
                  Causing this layer to trivially perform the an identity transform.
        !*/

        alias_tensor_instance get_gamma();
        /*!
            ensures
                - returns the gamma parameter that defines the behavior of forward().
        !*/

        alias_tensor_const_instance get_gamma() const;
        /*!
            ensures
                - returns the gamma parameter that defines the behavior of forward().
        !*/

        alias_tensor_instance get_beta();
        /*!
            ensures
                - returns the beta parameter that defines the behavior of forward().
        !*/

        alias_tensor_const_instance get_beta() const;
        /*!
            ensures
                - returns the beta parameter that defines the behavior of forward().
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the
            EXAMPLE_COMPUTATIONAL_LAYER_ interface.  Also note that get_layer_params()
            always returns an empty tensor since there are no learnable parameters in this
            object.
        !*/

    };

    template <typename SUBNET>
    using affine = add_layer<affine_, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        long _nr,
        long _nc,
        int _stride_y,
        int _stride_x,
        int _padding_y = _stride_y!=1? 0 : _nr/2,
        int _padding_x = _stride_x!=1? 0 : _nc/2
        >
    class max_pool_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - _nr >= 0
                - _nc >= 0
                - _stride_y > 0
                - _stride_x > 0
                - _padding_y >= 0
                - _padding_x >= 0
                - if (_nr != 0) then
                    - _padding_y < _nr
                - else
                    - _padding_y == 0
                - if (_nc != 0) then
                    - _padding_x < _nr
                - else
                    - _padding_x == 0

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a max pooling layer that takes an
                input tensor and downsamples it.  It does this by sliding a window over the
                images in an input tensor and outputting, for each channel, the maximum
                element within the window.  

                If _nr == 0 then it means the filter size covers all the rows in the input
                tensor, similarly for the _nc parameter.  To be precise, if we call the
                input tensor IN and the output tensor OUT, then OUT is defined as follows:
                    - let FILT_NR == (nr()==0) ? IN.nr() : nr()
                    - let FILT_NC == (nc()==0) ? IN.nc() : nc()
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k()
                    - OUT.nr() == 1+(IN.nr() + 2*padding_y() - FILT_NR)/stride_y()
                    - OUT.nc() == 1+(IN.nc() + 2*padding_x() - FILT_NC)/stride_x()
                    - for all valid s, k, r, and c:
                        - image_plane(OUT,s,k)(r,c) == max(subm_clipped(image_plane(IN,s,k),
                                                                  centered_rect(x*stride_x() + FILT_NC/2 - padding_x(),
                                                                                y*stride_y() + FILT_NR/2 - padding_y(),
                                                                                FILT_NC,
                                                                                FILT_NR)))
        !*/

    public:

        max_pool_ (
        );
        /*!
            ensures
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
        !*/

        long nr(
        ) const; 
        /*!
            ensures
                - returns the number of rows in the pooling window or 0 if the window size
                  is "the entire input tensor".
        !*/

        long nc(
        ) const;
        /*!
            ensures
                - returns the number of rows in the pooling window or 0 if the window size
                  is "the entire input tensor".
        !*/

        long stride_y(
        ) const; 
        /*!
            ensures
                - returns the vertical stride used when scanning the max pooling window
                  over an image.  That is, each window will be moved stride_y() pixels down
                  at a time when it moves over the image.
        !*/

        long stride_x(
        ) const;
        /*!
            ensures
                - returns the horizontal stride used when scanning the max pooling window
                  over an image.  That is, each window will be moved stride_x() pixels down
                  at a time when it moves over the image.
        !*/

        long padding_y(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the top and bottom
                  sides of the image.
        !*/

        long padding_x(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the left and right 
                  sides of the image.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& computed_output, const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <
        long nr,
        long nc,
        int stride_y,
        int stride_x,
        typename SUBNET
        >
    using max_pool = add_layer<max_pool_<nr,nc,stride_y,stride_x>, SUBNET>;

    template <
        typename SUBNET
        >
    using max_pool_everything = add_layer<max_pool_<0,0,1,1>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        long _nr,
        long _nc,
        int _stride_y,
        int _stride_x,
        int _padding_y = _stride_y!=1? 0 : _nr/2,
        int _padding_x = _stride_x!=1? 0 : _nc/2
        >
    class avg_pool_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - _nr >= 0
                - _nc >= 0
                - _stride_y > 0
                - _stride_x > 0
                - _padding_y >= 0
                - _padding_x >= 0
                - if (_nr != 0) then
                    - _padding_y < _nr
                - else
                    - _padding_y == 0
                - if (_nc != 0) then
                    - _padding_x < _nr
                - else
                    - _padding_x == 0

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines an average pooling layer that
                takes an input tensor and downsamples it.  It does this by sliding a window
                over the images in an input tensor and outputting, for each channel, the
                average element within the window.  

                If _nr == 0 then it means the filter size covers all the rows in the input
                tensor, similarly for the _nc parameter.  To be precise, if we call the
                input tensor IN and the output tensor OUT, then OUT is defined as follows:
                    - let FILT_NR == (nr()==0) ? IN.nr() : nr()
                    - let FILT_NC == (nc()==0) ? IN.nc() : nc()
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k()
                    - OUT.nr() == 1+(IN.nr() + 2*padding_y() - FILT_NR)/stride_y()
                    - OUT.nc() == 1+(IN.nc() + 2*padding_x() - FILT_NC)/stride_x()
                    - for all valid s, k, r, and c:
                        - image_plane(OUT,s,k)(r,c) == mean(subm_clipped(image_plane(IN,s,k),
                                                                  centered_rect(x*stride_x() + FILT_NC/2 - padding_x(),
                                                                                y*stride_y() + FILT_NR/2 - padding_y(),
                                                                                FILT_NC,
                                                                                FILT_NR)))
        !*/

    public:

        avg_pool_ (
        );
        /*!
            ensures
                - #nr() == _nr
                - #nc() == _nc
                - #stride_y() == _stride_y
                - #stride_x() == _stride_x
                - #padding_y() == _padding_y
                - #padding_x() == _padding_x
        !*/

        long nr(
        ) const; 
        /*!
            ensures
                - returns the number of rows in the pooling window or 0 if the window size
                  is "the entire input tensor".
        !*/

        long nc(
        ) const;
        /*!
            ensures
                - returns the number of rows in the pooling window or 0 if the window size
                  is "the entire input tensor".
        !*/

        long stride_y(
        ) const; 
        /*!
            ensures
                - returns the vertical stride used when scanning the pooling window
                  over an image.  That is, each window will be moved stride_y() pixels down
                  at a time when it moves over the image.
        !*/

        long stride_x(
        ) const;
        /*!
            ensures
                - returns the horizontal stride used when scanning the pooling window
                  over an image.  That is, each window will be moved stride_x() pixels down
                  at a time when it moves over the image.
        !*/

        long padding_y(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the top and bottom
                  sides of the image.
        !*/

        long padding_x(
        ) const; 
        /*!
            ensures
                - returns the number of pixels of zero padding added to the left and right 
                  sides of the image.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& computed_output, const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/

    };

    template <
        long nr,
        long nc,
        int stride_y,
        int stride_x,
        typename SUBNET
        >
    using avg_pool = add_layer<avg_pool_<nr,nc,stride_y,stride_x>, SUBNET>;

    template <
        typename SUBNET
        >
    using avg_pool_everything = add_layer<avg_pool_<0,0,1,1>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class relu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a rectified linear layer.
                Therefore, it passes its inputs through the function 
                    f(x)=max(x,0) 
                where f() is applied pointwise across the input tensor.
        !*/

    public:

        relu_(
        );

        void disable(
        );
        /*!
            ensures
                - #get_layer_params().size() == 0.
                - when forward_inplace and backward_inplace are called, they return immediately doing nothing.
                  Causing this layer to trivially perform the an identity transform.
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using relu = add_layer<relu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class prelu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a parametric rectified linear
                layer.  Therefore, it passes its inputs through the function 
                    f(x) = x>0 ? x : p*x 
                where f() is applied pointwise across the input tensor and p is a scalar
                parameter learned by this layer.


                This is the layer type introduced in the paper:
                    He, Kaiming, et al. "Delving deep into rectifiers: Surpassing
                    human-level performance on imagenet classification." Proceedings of the
                    IEEE International Conference on Computer Vision. 2015.
        !*/

    public:

        explicit prelu_(
            float initial_param_value = 0.25
        );
        /*!
            ensures
                - The p parameter will be initialized with initial_param_value.
                - #get_initial_param_value() == initial_param_value.
        !*/

        float get_initial_param_value (
        ) const;
        /*!
            ensures
                - returns the initial value of the prelu parameter. 
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using prelu = add_layer<prelu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class leaky_relu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a leaky rectified linear
                layer.  Therefore, it passes its inputs through the function
                    f(x) = x>0 ? x : alpha*x
                where f() is applied pointwise across the input tensor and alpha is a
                non-learned scalar.

                This is the layer type introduced in the paper:
                    A. L. Maas, A. Y. Hannun, and A. Y. Ng. "Rectifier nonlinearities improve
                    neural network acoustic models". In ICML, 2013.
        !*/

    public:
        explicit leaky_relu_(
            float alpha = 0.01f
        );
        /*!
            ensures
                - the alpha parameter will be initialized with the alpha value
        !*/

        float get_alpha(
        ) const;
        /*!
            ensures
                - returns the alpha parameter of the leaky_relu
        !*/

        template <typename SUBNET> void setup(const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using leaky_relu = add_layer<prelu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class sig_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a sigmoid layer.  Therefore, it
                passes its inputs through the function 
                    f(x)=1/(1+exp(-x)) 
                where f() is applied pointwise across the input tensor.
        !*/

    public:

        sig_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using sig = add_layer<sig_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class mish_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a mish layer.  Therefore, it
                passes its inputs through the function
                    f(x)= x*tanh(log(1+exp(x)))
                where f() is applied pointwise across the input tensor.

                This is the layer type introduced in the paper:
                Diganta Misra. "Mish: A Self Regularized Non-Monotonic Activation Function"
        !*/

    public:

        mish_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& data_output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor&);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using mish = add_layer<mish_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class htan_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a hyperbolic tangent layer.
                Therefore, it passes its inputs through the function 
                    f(x)=std::tanh(x)
                where f() is applied pointwise across the input tensor.
        !*/

    public:

        htan_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using htan = add_layer<htan_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class clipped_relu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a clipped version of the relu layer.
                Therefore, it passes its inputs through the function
                    f(x) = min(max(x, 0), ceiling)
                where f() is applied pointwise across the input tensor and ceiling is a
                non-learned scalar.
        !*/

    public:

        clipped_relu_(
            const float ceiling = 6.0f
        );
        /*!
            ensures
                - the ceiling parameter will be initialized with the ceiling value
        !*/

        float get_ceiling() const;
        /*!
            ensures
                - returns the celiling parameter of the clipped_relu
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using clipped_relu = add_layer<clipped_relu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class elu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines an exponential linear unit.
                Therefore, it passes its inputs through the function
                    f(x) = x>0 ? x : alpha*(exp(x)-1)
                where f() is applied pointwise across the input tensor and alpha is a
                non-learned scalar.

                This is the layer type introduced in the paper:
                Djork-Arné Clevert, Thomas Unterthiner, Sepp Hochreiter.
                "Fast and Accurate Deep Network Learning by Exponential Linear Units (ELUs)".
        !*/

    public:

        elu_(
            const float alpha = 1.0f
        );
        /*!
            ensures
                - the alpha parameter will be initialized with the alpha value
        !*/

        float get_alpha() const;
        /*!
            ensures
                - returns the alpha parameter of the elu
        !*/
        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using elu = add_layer<elu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class gelu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a gelu layer.  Therefore, it
                passes its inputs through the function
                        f(x)= x/2 * (1 + erf(x/sqrt(2))
                where f() is applied pointwise across the input tensor.

                This is the layer type introduced in the paper:
                Dan Hendrycks, Kevin Gimpel. "Gaussian Error Linear Units (GELUs)".
        !*/

    public:

        gelu_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& data_output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor&);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using gelu = add_layer<gelu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class smelu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a smooth rectified linear
                layer.  Therefore, it passes its inputs through the function f(x):
                    - if (x > beta) 1
                    - if (x < -beta) 0
                    - else std::pow(x + beta, 2) / (4 * beta)
                where f() is applied pointwise across the input tensor and beta is a
                non-learned scalar.

                This is the layer type introduced in the paper:
                "Smooth activations and reproducibility in deep networks" by
                Gil I. Shamir, Dong Lin, Lorenzo Coviello (https://arxiv.org/abs/2010.09931)
        !*/

    public:
        explicit smelu_(
            float beta = 1
        );
        /*!
            ensures
                - the beta parameter will be initialized with the beta value
        !*/

        float get_beta(
        ) const;
        /*!
            ensures
                - returns the beta parameter of the smelu
        !*/

        template <typename SUBNET> void setup(const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using smelu = add_layer<prelu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class silu_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a silu layer.  Therefore, it
                passes its inputs through the function
                        f(x)= x * sigmoid(x) = x / (1 + exp(-x))
                where f() is applied pointwise across the input tensor.

                This is the layer type introduced in the paper:
                Dan Hendrycks, Kevin Gimpel. "Gaussian Error Linear Units (GELUs)".
        !*/

    public:

        silu_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& data_output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor&);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using silu = add_layer<silu_, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <operation_mode s_mode_>
    class softmax_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above. It defines a softmax layer with two modes of operation:
                channel-wise and plane-wise.

                The softmax function s(x) is defined as:
                    s(x) == exp(x)/sum(exp(x))
                where x is a vector.

                1. Channel-wise mode (s_mode_ == CHANNEL_WISE):
                This mode treats the input tensor as a collection of multi-channel images
                and applies s() to each spatial location in each image. The tensor::k()
                channel elements at each position are input to s() and then replaced by
                the outputs of s().

                2. Plane-wise mode (s_mode_ == PLANE_WISE):
                This mode applies the softmax function across entire planes (nr x nc) of
                the input tensor, useful for operations in Large Language Models (LLMs)
                and other applications requiring 2D tensor processing.

                In both modes, the sum of the outputs of s() will always be equal to 1 for
                each application of the function.

            TEMPLATE PARAMETERS
                - s_mode_: Determines the mode of operation (CHANNEL_WISE or PLANE_WISE)
        !*/

    public:
        softmax_();

        template <typename SUBNET> void setup(const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(
            const tensor& computed_output,
            const tensor& gradient_input,
            tensor& data_grad,
            tensor& params_grad
        );
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_
            interface. Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/

        friend void serialize(const softmax_& item, std::ostream& out);
        friend void deserialize(softmax_& item, std::istream& in);
        friend std::ostream& operator<<(std::ostream& out, const softmax_& item);
        friend void to_xml(const softmax_& item, std::ostream& out);
    };

    template <typename SUBNET>
    using softmax = add_layer<softmax_<operation_mode::CHANNEL_WISE>, SUBNET>;

    template <typename SUBNET>
    using softmaxm = add_layer<softmax_<operation_mode::PLANE_WISE>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class softmax_all_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, it defines a softmax layer.  To be precise,
                we define the softmax function s(x) as:
                    s(x) == exp(x)/sum(exp(x)) 
                where x is a vector.  Then this layer treats its input tensor as a
                collection of tensor::num_samples() vectors and applies s() to each vector
                in the tensor.  Therefore, there are logically tensor::num_samples()
                invocations of s().
        !*/

    public:

        softmax_all_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ 
            interface.  Note that this layer doesn't have any parameters, so the tensor
            returned by get_layer_params() is always empty.
        !*/
    };

    template <typename SUBNET>
    using softmax_all = add_layer<softmax_all_, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class add_prev_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer simply adds the output of two previous layers.
                In particular, it adds the tensor from its immediate predecessor layer,
                sub.get_output(), with the tensor from a deeper layer,
                layer<tag>(sub).get_output().

                Therefore, you supply a tag via add_prev_'s template argument that tells it
                what layer to add to the output of the previous layer.  The result of this
                addition is output by add_prev_.  Finally, the addition happens pointwise
                according to 4D tensor arithmetic.  If the dimensions don't match then
                missing elements are presumed to be equal to 0.  Moreover, each dimension
                of the output tensor is equal to the maximum dimension of either of the
                inputs.  That is, if the tensors A and B are being added to produce C then:
                    - C.num_samples() == max(A.num_samples(), B.num_samples())
                    - C.k()  == max(A.k(), B.k())
                    - C.nr() == max(A.nr(), B.nr())
                    - C.nc() == max(A.nc(), B.nc())
        !*/

    public:
        add_prev_(
        ); 

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    template <
        template<typename> class tag,
        typename SUBNET
        >
    using add_prev = add_layer<add_prev_<tag>, SUBNET>;

    // Here we add some convenient aliases for using add_prev_ with the tag layers. 
    template <typename SUBNET> using add_prev1  = add_prev<tag1, SUBNET>;
    template <typename SUBNET> using add_prev2  = add_prev<tag2, SUBNET>;
    template <typename SUBNET> using add_prev3  = add_prev<tag3, SUBNET>;
    template <typename SUBNET> using add_prev4  = add_prev<tag4, SUBNET>;
    template <typename SUBNET> using add_prev5  = add_prev<tag5, SUBNET>;
    template <typename SUBNET> using add_prev6  = add_prev<tag6, SUBNET>;
    template <typename SUBNET> using add_prev7  = add_prev<tag7, SUBNET>;
    template <typename SUBNET> using add_prev8  = add_prev<tag8, SUBNET>;
    template <typename SUBNET> using add_prev9  = add_prev<tag9, SUBNET>;
    template <typename SUBNET> using add_prev10 = add_prev<tag10, SUBNET>;
    using add_prev1_  = add_prev_<tag1>;
    using add_prev2_  = add_prev_<tag2>;
    using add_prev3_  = add_prev_<tag3>;
    using add_prev4_  = add_prev_<tag4>;
    using add_prev5_  = add_prev_<tag5>;
    using add_prev6_  = add_prev_<tag6>;
    using add_prev7_  = add_prev_<tag7>;
    using add_prev8_  = add_prev_<tag8>;
    using add_prev9_  = add_prev_<tag9>;
    using add_prev10_ = add_prev_<tag10>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class mult_prev_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer simply multiplies the output of two previous
                layers.  In particular, it multiplies the tensor from its immediate
                predecessor layer, sub.get_output(), with the tensor from a deeper layer,
                layer<tag>(sub).get_output().

                Therefore, you supply a tag via mult_prev_'s template argument that tells
                it what layer to multiply with the output of the previous layer.  The
                result of this multiplication is output by mult_prev_.  Finally, the
                multiplication happens pointwise according to 4D tensor arithmetic.  If the
                dimensions don't match then missing elements are presumed to be equal to 0.
                Moreover, each dimension of the output tensor is equal to the maximum
                dimension of either of the inputs.  That is, if the tensors A and B are
                being multiplied to produce C then:
                    - C.num_samples() == max(A.num_samples(), B.num_samples())
                    - C.k()  == max(A.k(), B.k())
                    - C.nr() == max(A.nr(), B.nr())
                    - C.nc() == max(A.nc(), B.nc())
        !*/

    public:
        mult_prev_(
        ); 

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    template <
        template<typename> class tag,
        typename SUBNET
        >
    using mult_prev = add_layer<mult_prev_<tag>, SUBNET>;

    // Here we add some convenient aliases for using mult_prev_ with the tag layers. 
    template <typename SUBNET> using mult_prev1  = mult_prev<tag1, SUBNET>;
    template <typename SUBNET> using mult_prev2  = mult_prev<tag2, SUBNET>;
    template <typename SUBNET> using mult_prev3  = mult_prev<tag3, SUBNET>;
    template <typename SUBNET> using mult_prev4  = mult_prev<tag4, SUBNET>;
    template <typename SUBNET> using mult_prev5  = mult_prev<tag5, SUBNET>;
    template <typename SUBNET> using mult_prev6  = mult_prev<tag6, SUBNET>;
    template <typename SUBNET> using mult_prev7  = mult_prev<tag7, SUBNET>;
    template <typename SUBNET> using mult_prev8  = mult_prev<tag8, SUBNET>;
    template <typename SUBNET> using mult_prev9  = mult_prev<tag9, SUBNET>;
    template <typename SUBNET> using mult_prev10 = mult_prev<tag10, SUBNET>;
    using mult_prev1_  = mult_prev_<tag1>;
    using mult_prev2_  = mult_prev_<tag2>;
    using mult_prev3_  = mult_prev_<tag3>;
    using mult_prev4_  = mult_prev_<tag4>;
    using mult_prev5_  = mult_prev_<tag5>;
    using mult_prev6_  = mult_prev_<tag6>;
    using mult_prev7_  = mult_prev_<tag7>;
    using mult_prev8_  = mult_prev_<tag8>;
    using mult_prev9_  = mult_prev_<tag9>;
    using mult_prev10_ = mult_prev_<tag10>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class multm_prev_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above. This layer performs matrix multiplication on the output
                of two previous layers. It multiplies the tensor from its immediate
                predecessor layer, sub.get_output(), with the tensor from a deeper layer,
                layer<tag>(sub).get_output().

                The tag template argument specifies which layer to multiply with the
                output of the previous layer. The result of this multiplication is
                output by multm_prev_. The multiplication is performed using a modified
                version of gemm() to account for the 2D matrix dimension in the nr()xnc()
                planes of Dlib's 4D tensors.

                This layer is similar to mult_prev_, but it considers the full matrix
                in the nr()xnc() planes of the tensor, rather than just the upper
                num_samples()xk() plane. This makes it suitable for implementing
                mechanisms like attention, especially when the k() channel plane is
                used to model multiple heads for parallel matrix processing.

                The output tensor dimensions are determined as follows:
                    - output.num_samples() == t1.num_samples()
                    - output.k() == t1.k()
                    - output.nr() == t1.nr()
                    - output.nc() == t2.nc()
                where t1 is sub.get_output() and t2 is layer<tag>(sub).get_output().
        !*/

    public:
        multm_prev_(
        ); 

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;        
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        template<typename> class tag,
        typename SUBNET
        >
    using multm_prev = add_layer<multm_prev_<tag>, SUBNET>;

    // Here we add some convenient aliases for using multm_prev_ with the tag layers. 
    template <typename SUBNET> using multm_prev1  = multm_prev<tag1, SUBNET>;
    template <typename SUBNET> using multm_prev2  = multm_prev<tag2, SUBNET>;
    template <typename SUBNET> using multm_prev3  = multm_prev<tag3, SUBNET>;
    template <typename SUBNET> using multm_prev4  = multm_prev<tag4, SUBNET>;
    template <typename SUBNET> using multm_prev5  = multm_prev<tag5, SUBNET>;
    template <typename SUBNET> using multm_prev6  = multm_prev<tag6, SUBNET>;
    template <typename SUBNET> using multm_prev7  = multm_prev<tag7, SUBNET>;
    template <typename SUBNET> using multm_prev8  = multm_prev<tag8, SUBNET>;
    template <typename SUBNET> using multm_prev9  = multm_prev<tag9, SUBNET>;
    template <typename SUBNET> using multm_prev10 = multm_prev<tag10, SUBNET>;
    using multm_prev1_  = multm_prev_<tag1>;
    using multm_prev2_  = multm_prev_<tag2>;
    using multm_prev3_  = multm_prev_<tag3>;
    using multm_prev4_  = multm_prev_<tag4>;
    using multm_prev5_  = multm_prev_<tag5>;
    using multm_prev6_  = multm_prev_<tag6>;
    using multm_prev7_  = multm_prev_<tag7>;
    using multm_prev8_  = multm_prev_<tag8>;
    using multm_prev9_  = multm_prev_<tag9>;
    using multm_prev10_ = multm_prev_<tag10>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class resize_prev_to_tagged_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer resizes the output channels of the previous layer
                to have the same number of rows and columns as the output of the tagged layer.

                This layer uses bilinear interpolation. If the sizes match already, then it
                simply copies the data.

                Therefore, you supply a tag via resize_prev_to_tagged's template argument that
                tells it what layer to use for the target size.

                If tensor PREV is resized to size of tensor TAGGED, then a tensor OUT is
                produced such that:
                    - OUT.num_samples() == PREV.num_samples()
                    - OUT.k()  == PREV.k()
                    - OUT.nr() == TAGGED.nr()
                    - OUT.nc() == TAGGED.nc()
        !*/

    public:
        resize_prev_to_tagged_(
        ); 

        template <typename SUBNET> void setup(const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    template <
        template<typename> class tag,
        typename SUBNET
        >
    using resize_prev_to_tagged = add_layer<resize_prev_to_tagged_<tag>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class scale_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer scales the output channels of the tagged layer
                by multiplying it with the output of the previous layer.  To be specific:
                    - Let INPUT  == layer<tag>(sub).get_output()
                    - Let SCALES == sub.get_output()
                    - This layer takes INPUT and SCALES as input.
                    - The output of this layer has the same dimensions as INPUT.
                    - This layer requires:
                        - SCALES.num_samples() == INPUT.num_samples()
                        - SCALES.k()  == INPUT.k()
                        - SCALES.nr() == 1
                        - SCALES.nc() == 1
                    - The output tensor is produced by pointwise multiplying SCALES with
                      INPUT at each spatial location.  Therefore, if OUT is the output of
                      this layer then we would have:
                        OUT(n,k,r,c) == INPUT(n,k,r,c)*SCALES(n,k)
        !*/

    public:
        scale_(
        ); 

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    template <
        template<typename> class tag,
        typename SUBNET
        >
    using scale = add_layer<scale_<tag>, SUBNET>;

    // Here we add some convenient aliases for using scale_ with the tag layers. 
    template <typename SUBNET> using scale1  = scale<tag1, SUBNET>;
    template <typename SUBNET> using scale2  = scale<tag2, SUBNET>;
    template <typename SUBNET> using scale3  = scale<tag3, SUBNET>;
    template <typename SUBNET> using scale4  = scale<tag4, SUBNET>;
    template <typename SUBNET> using scale5  = scale<tag5, SUBNET>;
    template <typename SUBNET> using scale6  = scale<tag6, SUBNET>;
    template <typename SUBNET> using scale7  = scale<tag7, SUBNET>;
    template <typename SUBNET> using scale8  = scale<tag8, SUBNET>;
    template <typename SUBNET> using scale9  = scale<tag9, SUBNET>;
    template <typename SUBNET> using scale10 = scale<tag10, SUBNET>;
    using scale1_  = scale_<tag1>;
    using scale2_  = scale_<tag2>;
    using scale3_  = scale_<tag3>;
    using scale4_  = scale_<tag4>;
    using scale5_  = scale_<tag5>;
    using scale6_  = scale_<tag6>;
    using scale7_  = scale_<tag7>;
    using scale8_  = scale_<tag8>;
    using scale9_  = scale_<tag9>;
    using scale10_ = scale_<tag10>;

// ----------------------------------------------------------------------------------------

    template <
        template<typename> class tag
        >
    class scale_prev_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer scales the output channels of the tagged layer
                by multiplying it with the output of the previous layer.  It is excatly the
                same as the scale_ layer, but with the inputs swapped, which is useful since
                it allows mapping between inputs and outputs of this layer.  To be specific:
                    - Let INPUT == sub.get_output()
                    - Let SCALES == layer<tag>(sub).get_output()
                    - This layer takes INPUT and SCALES as input.
                    - The output of this layer has the same dimensions as INPUT.
                    - This layer requires:
                        - SCALES.num_samples() == INPUT.num_samples()
                        - SCALES.k()  == INPUT.k()
                        - SCALES.nr() == 1
                        - SCALES.nc() == 1
                    - The output tensor is produced by pointwise multiplying SCALES with
                      INPUT at each spatial location.  Therefore, if OUT is the output of
                      this layer then we would have:
                        OUT(n,k,r,c) == INPUT(n,k,r,c)*SCALES(n,k)
        !*/

    public:
        scale_prev_(
        );

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    template <
        template<typename> class tag,
        typename SUBNET
        >
    using scale_prev = add_layer<scale_prev_<tag>, SUBNET>;

    // Here we add some convenient aliases for using scale_prev_ with the tag layers.
    template <typename SUBNET> using scale_prev1  = scale_prev<tag1, SUBNET>;
    template <typename SUBNET> using scale_prev2  = scale_prev<tag2, SUBNET>;
    template <typename SUBNET> using scale_prev3  = scale_prev<tag3, SUBNET>;
    template <typename SUBNET> using scale_prev4  = scale_prev<tag4, SUBNET>;
    template <typename SUBNET> using scale_prev5  = scale_prev<tag5, SUBNET>;
    template <typename SUBNET> using scale_prev6  = scale_prev<tag6, SUBNET>;
    template <typename SUBNET> using scale_prev7  = scale_prev<tag7, SUBNET>;
    template <typename SUBNET> using scale_prev8  = scale_prev<tag8, SUBNET>;
    template <typename SUBNET> using scale_prev9  = scale_prev<tag9, SUBNET>;
    template <typename SUBNET> using scale_prev10 = scale_prev<tag10, SUBNET>;
    using scale_prev1_  = scale_prev_<tag1>;
    using scale_prev2_  = scale_prev_<tag2>;
    using scale_prev3_  = scale_prev_<tag3>;
    using scale_prev4_  = scale_prev_<tag4>;
    using scale_prev5_  = scale_prev_<tag5>;
    using scale_prev6_  = scale_prev_<tag6>;
    using scale_prev7_  = scale_prev_<tag7>;
    using scale_prev8_  = scale_prev_<tag8>;
    using scale_prev9_  = scale_prev_<tag9>;
    using scale_prev10_ = scale_prev_<tag10>;

    // ----------------------------------------------------------------------------------------

    template<
        template<typename> class... TAG_TYPES
        >
    class concat_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  This layer simply concatenates the output of tagged layers.
                Importantly, each input layer must have the same dimensions (i.e.
                num_samples, nr, and nc) except for the k channel, which may vary.  This is
                because the concatenation happens along the k dimension.  That is, the
                output of this network is a tensor, OUT, that is the concatenation of the
                tensors:
                    for each (tag in TAG_TYPES)
                        layer<tag>(subnet).get_output()
                Therefore, out.num_samples(), out.nr(), and out.nc() match the dimensions
                of the input tensors while OUT.k() is the sum of the input layer's k()
                dimensions.
        !*/

    public:
        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output(dpoint p) const;
        dpoint map_output_to_input(dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };


    // concat layer definitions
    template <template<typename> class TAG1,
              template<typename> class TAG2,
              typename SUBNET>
    using concat2 = add_layer<concat_<TAG1, TAG2>, SUBNET>;

    template <template<typename> class TAG1,
              template<typename> class TAG2,
              template<typename> class TAG3,
              typename SUBNET>
    using concat3 = add_layer<concat_<TAG1, TAG2, TAG3>, SUBNET>;

    template <template<typename> class TAG1,
              template<typename> class TAG2,
              template<typename> class TAG3,
              template<typename> class TAG4,
              typename SUBNET>
    using concat4 = add_layer<concat_<TAG1, TAG2, TAG3, TAG4>, SUBNET>;

    template <template<typename> class TAG1,
              template<typename> class TAG2,
              template<typename> class TAG3,
              template<typename> class TAG4,
              template<typename> class TAG5,
              typename SUBNET>
    using concat5 = add_layer<concat_<TAG1, TAG2, TAG3, TAG4, TAG5>, SUBNET>;

// ----------------------------------------------------------------------------------------
    
    /*!A inception layer definitions !*/

    // Now define inception layer tag types.  These layer aliases allow creating
    // the networks described in the paper: 
    //   Szegedy, Christian, et al. "Going deeper with convolutions." Proceedings of
    //   the IEEE Conference on Computer Vision and Pattern Recognition. 2015.
    // See the dnn_inception_ex.cpp example for a complete example of their use.  Note also
    // that we use tag ID numbers >= 1000 to avoid conflict with user's tag layers.
    template <typename SUBNET> using itag0  = add_tag_layer< 1000 + 0, SUBNET>;
    template <typename SUBNET> using itag1  = add_tag_layer< 1000 + 1, SUBNET>;
    template <typename SUBNET> using itag2  = add_tag_layer< 1000 + 2, SUBNET>;
    template <typename SUBNET> using itag3  = add_tag_layer< 1000 + 3, SUBNET>;
    template <typename SUBNET> using itag4  = add_tag_layer< 1000 + 4, SUBNET>;
    template <typename SUBNET> using itag5  = add_tag_layer< 1000 + 5, SUBNET>;
    // skip to inception input
    template <typename SUBNET> using iskip  = add_skip_layer< itag0, SUBNET>;

    // here are some templates to be used for creating inception layer groups
    template <template<typename>class B1,
              template<typename>class B2,
              typename SUBNET>
    using inception2 = concat2<itag1, itag2, itag1<B1<iskip< itag2<B2< itag0<SUBNET>>>>>>>;

    template <template<typename>class B1,
              template<typename>class B2,
              template<typename>class B3,
              typename SUBNET>
    using inception3 = concat3<itag1, itag2, itag3, itag1<B1<iskip< itag2<B2<iskip< itag3<B3<  itag0<SUBNET>>>>>>>>>>;

    template <template<typename>class B1,
              template<typename>class B2,
              template<typename>class B3,
              template<typename>class B4,
              typename SUBNET>
    using inception4 = concat4<itag1, itag2, itag3, itag4,
                itag1<B1<iskip< itag2<B2<iskip< itag3<B3<iskip<  itag4<B4<  itag0<SUBNET>>>>>>>>>>>>>;

    template <template<typename>class B1,
              template<typename>class B2,
              template<typename>class B3,
              template<typename>class B4,
              template<typename>class B5,
              typename SUBNET>
    using inception5 = concat5<itag1, itag2, itag3, itag4, itag5,
                itag1<B1<iskip< itag2<B2<iskip< itag3<B3<iskip<  itag4<B4<iskip<  itag5<B5<  itag0<SUBNET>>>>>>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------

    const double DEFAULT_L2_NORM_EPS = 1e-5;

    class l2normalize_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  It takes tensors as input and L2 normalizes them.  In particular,
                it has the following properties:
                    - The output tensors from this layer have the same dimensions as the
                      input tensors.
                    - If you think of each input tensor as a set of tensor::num_samples()
                      vectors, then the output tensor contains the same vectors except they
                      have been length normalized so that their L2 norms are all 1.  I.e. 
                      for each vector v we will have ||v||==1.
        !*/

    public:

        explicit l2normalize_(
            double eps = tt::DEFAULT_L2_NORM_EPS
        );
        /*!
            requires
                - eps > 0
            ensures
                - #get_eps() == eps
        !*/

        double get_eps(
        ) const; 
        /*!
            ensures
                - When we normalize a vector we divide it by its L2 norm.  However, the
                  get_eps() value is added to the squared norm prior to division to avoid
                  ever dividing by zero. 
        !*/

        template <typename SUBNET> void setup (const SUBNET& sub);
        void forward_inplace(const tensor& input, tensor& output);
        void backward_inplace(const tensor& computed_output, const tensor& gradient_input, tensor& data_grad, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

// ----------------------------------------------------------------------------------------

    template <
        long _offset,
        long _k,
        long _nr,
        long _nc
        >
    class extract_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - 0 <= _offset
                - 0 < _k
                - 0 < _nr
                - 0 < _nc

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, the output of this layer is simply a copy of
                the input tensor.  However, you can configure the extract layer to output
                only some subset of the input tensor and also to reshape it.  Therefore,
                the dimensions of the tensor output by this layer are as follows (letting
                IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == _k 
                    - OUT.nr() == _nr 
                    - OUT.nc() == _nc 

                So the output will always have the same number of samples as the input, but
                within each sample (the k,nr,nc part) we will copy only a subset of the
                values.  Moreover, the _offset parameter controls which part of each sample
                we take.  To be very precise, we will have:
                    - let IN_SIZE   = IN.k()*IN.nr()*IN.nc()
                    - let OUT_SIZE  = _k*_nr*_nc 
                    - for i in range[0,IN.num_samples()) and j in range[0,OUT_SIZE):
                        - OUT.host()[i*OUT_SIZE+j] == IN.host()[i*IN_SIZE+_offset+j]


                Finally, all this means that the input tensor to this layer must have a big
                enough size to accommodate taking a _k*_nr*_nc slice from each of its
                samples.  
        !*/

    public:

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        long offset,
        long k,
        long nr,
        long nc,
        typename SUBNET
        >
    using extract = add_layer<extract_<offset,k,nr,nc>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        long _offset_k,
        long _offset_nr,
        long _offset_nc,
        long _k,
        long _nr,
        long _nc
        >
    class slice_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - 0 <= _offset_k
                - 0 <= _offset_nr
                - 0 <= _offset_nc
                - 0 < _k
                - 0 < _nr
                - 0 < _nc

            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above.  In particular, the output of this layer is simply a copy of
                the input tensor. It is similar to extract in that you can configure the
                slice layer to output only some subset of the input tensor, but slice allows
                copies of non-contiguous regions of the input which enables three dimensional
                cropping of a tensor. The dimensions of the tensor output by this layer
                are as follows (letting IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == _k 
                    - OUT.nr() == _nr 
                    - OUT.nc() == _nc 

                So the output will always have the same number of samples as the input, but
                within each sample (the k,nr,nc part) we will copy only a subset of the
                values. Moreover, the _offset_k, _offset_nr, and _offset_nc parameters
                control which channels, rows, and columns of each sample we take.
                To be very precise, we will have:
                    - let IN_SIZE   = IN.k()*IN.nr()*IN.nc()
                    - let OUT_SIZE  = _k*_nr*_nc 
                    - for i in range[0,IN.num_samples()) and j in range[0,OUT_SIZE):
                        - let k = (j / (OUT.nr()*OUT.nc())) % OUT.k()
                        - let r = (j / OUT.nc()) % IN.nr()
                        - let c = j % OUT.nc()
                        - OUT.host()[i*OUT_SIZE+j] == IN.host()[i*IN_SIZE+
                                                                k_stride*(_offset_k+k)+
                                                                row_stride*(_offset_nr+r)+
                                                                col_stride*(_offset_nc+c)]


                Finally, all this means that the input tensor to this layer must have a big
                enough size to accommodate taking a _k*_nr*_nc slice from each of its
                samples.  
        !*/

    public:

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        long offset_k,
        long offset_nr,
        long offset_nc,
        long k,
        long nr,
        long nc,
        typename SUBNET
        >
    using slice = add_layer<slice_<offset_k,offset_nr,offset_nc,k,nr,nc>, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <long long row_stride = 2, long long col_stride = 2>
    class reorg_
    {
        /*!
            REQUIREMENTS ON TEMPLATE ARGUMENTS
                - row_stride >= 1
                - col_stride >= 1

            WHAT THIS OBJECT REPRESENTS
                This class implements the EXAMPLE_COMPUTATIONAL_LAYER_ interface, performing a 
                reorganization of tensor data. It rearranges spatial information along the channel
                dimension, effectively "folding" spatial dimensions into channels.
                
                The dimensions of the output tensor are as follows (letting IN be the input tensor
                and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k() * row_stride * col_stride
                    - OUT.nr() == IN.nr() / row_stride
                    - OUT.nc() == IN.nc() / col_stride

                Therefore, the output tensor maintains the same number of samples as the input but
                alters the channel and spatial dimensions based on the specified strides.
                
                Specifically, for all n, k, r, c in OUT:
                    OUT.host[tensor_index(OUT, n, k, r, c)] ==
                    IN.host[tensor_index(IN,
                                        n,
                                        k % IN.k(),
                                        r * row_stride + (k / IN.k()) / col_stride,
                                        c * col_stride + (k / IN.k()) % col_stride)]

                **Enhancement Note:**  
                The underlying utility functions (`reorg` and `reorg_gradient`) now include an
                optional `bool add_to` parameter. While the current implementation uses the default
                value to maintain existing behavior, this parameter allows for future reversible
                operations and gradient accumulation flexibility within neural network layers.

                You can think of this layer as an alternative to a strided convolutional layer for
                downsampling tensors, offering similar spatial reduction with different internal
                gradient propagation mechanics.
        !*/

    public:

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        dpoint map_input_to_output (dpoint p) const;
        dpoint map_output_to_input (dpoint p) const;
        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <typename SUBNET>
    using reorg = add_layer<reorg_<2, 2>, SUBNET>;

// ----------------------------------------------------------------------------------------

    class transpose_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface
                defined above. In particular, this layer performs a 2D matrix transposition
                on each of the k planes within each sample of a 4D tensor.

                The dimensions of the tensor output by this layer are as follows (letting
                IN be the input tensor and OUT the output tensor):
                    - OUT.num_samples() == IN.num_samples()
                    - OUT.k()  == IN.k()
                    - OUT.nr() == IN.nc()
                    - OUT.nc() == IN.nr()

                The transposition is performed as follows:
                    - For each sample i and each k-plane j:
                        - OUT[i][j][r][c] = IN[i][j][c][r] for all r in [0, IN.nc()) and c in [0, IN.nr())

                This layer does not have any learnable parameters.
        !*/

    public:

        transpose_() = default;

        template <typename SUBNET> void setup (const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        
        inline dpoint map_input_to_output(dpoint p) const;
        inline dpoint map_output_to_input(dpoint p) const;

        const tensor& get_layer_params() const; 
        tensor& get_layer_params(); 

        friend void serialize(const transpose_& item, std::ostream& out);
        friend void deserialize(transpose_& item, std::istream& in);

        friend std::ostream& operator<<(std::ostream& out, const transpose_& item);
        friend void to_xml(const transpose_& item, std::ostream& out);

        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    private:
        resizable_tensor params; // unused
    };

    template <typename SUBNET>
    using transpose = add_layer<transpose_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class positional_encodings_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
                It defines a positional encoding layer that adds position information to
                the input tensor. This is particularly useful in transformer architectures
                where the order of the sequence matters.

                The dimensions of the tensors output by this layer are the same as the input
                tensor dimensions.

                This implementation is based on the positional encoding described in:
                Vaswani, A., Shazeer, N., Parmar, N., Uszkoreit, J., Jones, L., Gomez, A. N., 
                Kaiser, Ł., & Polosukhin, I. (2017). Attention is all you need. In Advances 
                in neural information processing systems (pp. 5998-6008).

                The encoding uses sine and cosine functions of different frequencies:
                PE(pos, 2i)   = sin(pos / 10000^(2i/d_model))
                PE(pos, 2i+1) = cos(pos / 10000^(2i/d_model))
                where pos is the position and i is the dimension.
        !*/

    public:

        positional_encodings_(
            unsigned long sequence_dim_ = 1,
            unsigned long embedding_dim_ = 1
        );
        /*!
            ensures
                - #sequence_dim == sequence_dim_
                - #embedding_dim == embedding_dim_
        !*/

        positional_encodings_ (
            const positional_encodings_& item
        );
        /*!
            ensures
                - EXAMPLE_COMPUTATIONAL_LAYER_ objects are copy constructable
        !*/

        positional_encodings_& operator=(
            const positional_encodings_& item
        );
        /*!
            ensures
                - EXAMPLE_COMPUTATIONAL_LAYER_ objects are assignable
        !*/

        template <typename SUBNET>
        void setup (
            const SUBNET& sub
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
            ensures
                - performs any necessary setup for the layer, including the calculation
                of positional encodings based on the dimensions of the input.
        !*/

        template <typename SUBNET>
        void forward(
            const SUBNET& sub,
            resizable_tensor& output
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
            ensures
                - Adds the positional encodings to the output of the subnetwork and 
                stores the results into #output.
        !*/

        template <typename SUBNET>
        void backward(
            const tensor& gradient_input,
            SUBNET& sub,
            tensor& params_grad
        );
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of this file.
                - setup() has been called.
                - #params_grad is unused in this layer as there are no learnable parameters.
            ensures
                - Computes the gradient of the layer with respect to the input, which
                is simply the input gradient itself as positional encodings are constant.
        !*/

        const tensor& get_layer_params(
        ) const;
        /*!
            ensures
                - returns the parameters that define the behavior of forward().
                Note: This layer has no learnable parameters, so this returns an empty tensor.
        !*/

        tensor& get_layer_params(
        );
        /*!
            ensures
                - returns the parameters that define the behavior of forward().
                Note: This layer has no learnable parameters, so this returns an empty tensor.
        !*/

        const tensor& get_positional_encodings(
        ) const;
        /*!
            ensures
                - returns the computed positional encodings.
        !*/

        tensor& get_positional_encodings(
        );
        /*!
            ensures
                - returns the computed positional encodings.
        !*/

        friend void serialize(const positional_encodings_& item, std::ostream& out);
        friend void deserialize(positional_encodings_& item, std::istream& in);
        /*!
            provides serialization support
        !*/

        friend std::ostream& operator<<(std::ostream& out, const positional_encodings_& item);
        /*!
            print a string describing this layer.
        !*/

        friend void to_xml(const positional_encodings_& item, std::ostream& out);
        /*!
            This function is optional, but required if you want to print your networks with
            net_to_xml(). It prints a layer as XML.
        !*/
    };

    template <typename SUBNET>
    using positional_encodings = add_layer<positional_encodings_, SUBNET>;

// ----------------------------------------------------------------------------------------

    template <
        unsigned long num_embeddings_,
        unsigned long embedding_dim_
        >
    class embeddings_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object represents an embedding layer in a neural network. It maps discrete
                tokens to continuous vector representations. This is a fundamental technique in
                natural language processing and other domains dealing with categorical data.

                The layer takes as input a tensor of integer indices and outputs a tensor of 
                the same shape (except for the last dimension) where each index is replaced by 
                its corresponding embedding vector.

                For more information on embeddings, see:
                Mikolov, T., Sutskever, I., Chen, K., Corrado, G. S., & Dean, J. (2013). 
                Distributed representations of words and phrases and their compositionality. 
                In Advances in neural information processing systems (pp. 3111-3119).

            TEMPLATE PARAMETERS
                - num_embeddings_: The size of the embedding dictionary, i.e., the number of 
                                discrete tokens that can be embedded.
                - embedding_dim_: The dimensionality of each embedding vector.

            CONVENTION
                - get_embeddings() returns the tensor of embedding vectors.
                - get_num_embeddings() == num_embeddings_
                - get_embedding_dim() == embedding_dim_
                - get_learning_rate_multiplier() returns the learning rate multiplier for this layer.
                - get_scale_by_freq() returns whether to scale gradients by token frequency.
        */        
    public:
        embeddings_() = default;

        unsigned long get_num_embeddings() const;
        unsigned long get_embedding_dim() const;
        double get_learning_rate_multiplier() const;
        bool get_scale_by_freq() const;

        void set_num_embeddings(unsigned long num);
        void set_embedding_dim(unsigned long dim);
        void set_learning_rate_multiplier(double val);
        void set_scale_by_freq(bool val);

        template <typename SUBNET> void setup(const SUBNET& sub);
        template <typename SUBNET> void forward(const SUBNET& sub, resizable_tensor& output);
        template <typename SUBNET> void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);

        const tensor& get_layer_params() const;
        tensor& get_layer_params();
        const tensor& get_embeddings() const;
        tensor& get_embeddings();

        friend void serialize(const embeddings_& item, std::ostream& out);
        friend void deserialize(embeddings_& item, std::istream& in);
        friend std::ostream& operator<<(std::ostream& out, const embeddings_& item);
        friend void to_xml(const embeddings_& item, std::ostream& out);

        /*!
            These functions are implemented as described in the EXAMPLE_COMPUTATIONAL_LAYER_ interface.
        !*/
    };

    template <
        unsigned long num_embeddings,
        unsigned long embedding_dim,
        typename SUBNET
        >
    using embeddings = add_layer<embeddings_<num_embeddings, embedding_dim>, SUBNET>;

// ----------------------------------------------------------------------------------------

    struct neg_infinity_tag {};
    struct zero_tag {};

    template<typename T>
    struct is_special_value : std::false_type {};
    template<>
    struct is_special_value<neg_infinity_tag> : std::true_type {};
    template<>
    struct is_special_value<zero_tag> : std::true_type {};

    template<long diag_, typename tag_, long num_ = 0, long den_ = 1>
    class tril_
    {
        /*!
            TEMPLATE PARAMETERS
                - diag_: A long integer specifying the diagonal offset.
                - tag_: A type tag specifying special values or void for numeric values.
                - num_: Numerator for numeric diagonal value (default is 0, only used if tag_ is void).
                - den_: Denominator for numeric diagonal value (default is 1, only used if tag_ is void).

            REQUIREMENTS
                - diag_ must be an integer.
                - tag_ must be either neg_infinity_tag, zero_tag, or void.
                - If tag_ is void, num_ and den_ are used to compute the diagonal value.
                - If tag_ is neg_infinity_tag or zero_tag, num_ and den_ are ignored.

            WHAT THIS OBJECT REPRESENTS
                This object implements a layer in a deep neural network that applies a lower triangular mask to
                its input tensor. The mask is defined such that all elements above the specified diagonal are set
                to a given value. The diagonal offset and the mask value are determined by the template parameters.

            DIAGONAL VALUE DETERMINATION
                - If tag_ is neg_infinity_tag: diagonal value is set to negative infinity.
                - If tag_ is zero_tag: diagonal value is set to zero.
                - If tag_ is void: diagonal value is set to num_ / den_ as a float.

            DIAGONAL OFFSET
                The diag_ parameter determines the diagonal above which elements are masked:
                - diag_ = 0: main diagonal
                - diag_ > 0: diag_ steps above the main diagonal
                - diag_ < 0: |diag_| steps below the main diagonal

            EXAMPLE USAGE
                // Create a layer that masks all elements above the main diagonal with -inf
                tril_<0, neg_infinity_tag> layer1;

                // Create a layer that masks all elements above the main diagonal with 0
                tril_<0, zero_tag> layer2;

                // Create a layer that masks all elements above the main diagonal with 0.5
                tril_<0, void, 1, 2> layer3;

                // Create a layer that masks all elements 5 positions above the main diagonal with -inf
                tril_<5, neg_infinity_tag> layer4;

                // Create a layer that masks all elements 3 positions below the main diagonal with 0.25
                tril_<-3, void, 1, 4> layer5;

            SERIALIZATION SUPPORT
                This object supports serialization and deserialization via the serialize() and deserialize() functions.
        !*/

    public:
        tril_() = default;
        /*!
            ensures
                - This object is properly initialized.
        !*/

        template <typename SUBNET>
        void setup(const SUBNET& sub);
        /*!
            requires
                - SUBNET is a valid network layer type.
            ensures
                - Initializes the mask based on the dimensions of the input tensor from sub.
        !*/

        template <typename SUBNET>
        void forward(const SUBNET& sub, resizable_tensor& output);
        /*!
            requires
                - SUBNET is a valid network layer type.
            ensures
                - Applies the lower triangular mask to the input tensor from sub and stores the result in output.
        !*/

        template <typename SUBNET>
        void backward(const tensor& gradient_input, SUBNET& sub, tensor& params_grad);
        /*!
            requires
                - SUBNET is a valid network layer type.
            ensures
                - Computes the gradient of the loss with respect to the input tensor and stores it in sub.
        !*/

        inline dpoint map_input_to_output(const dpoint& p) const;
        /*!
            ensures
                - Maps a point from the input tensor to the corresponding point in the output tensor.
        !*/

        inline dpoint map_output_to_input(const dpoint& p) const;
        /*!
            ensures
                - Maps a point from the output tensor to the corresponding point in the input tensor.
        !*/

        const tensor& get_layer_params() const;
        /*!
            ensures
                - Returns the parameters of this layer.
        !*/

        tensor& get_layer_params();
        /*!
            ensures
                - Returns the parameters of this layer.
        !*/

        friend void serialize(const tril_& item, std::ostream& out);
        /*!
            ensures
                - Serializes the state of this object to the given output stream.
        !*/

        friend void deserialize(tril_& item, std::istream& in);
        /*!
            ensures
                - Deserializes the state of this object from the given input stream.
        !*/

        friend std::ostream& operator<<(std::ostream& out, const tril_& item);
        /*!
            ensures
                - Prints a human-readable representation of this object to the given output stream.
        !*/

        friend void to_xml(const tril_& item, std::ostream& out);
        /*!
            ensures
                - Serializes the state of this object to XML format and writes it to the given output stream.
        !*/
    };

    template <typename SUBNET>
    using tril = add_layer<tril_<0, zero_tag>, SUBNET>;

    template <typename SUBNET>
    using tril_mask = add_layer<tril_<0, neg_infinity_tag>, SUBNET>;

    template <long diag, long num, long den, typename SUBNET>
    using tril_diag = add_layer<tril_<diag, void, num, den>, SUBNET>;

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_DNn_LAYERS_ABSTRACT_H_


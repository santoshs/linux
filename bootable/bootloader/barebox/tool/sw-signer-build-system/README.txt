
    INTRODUCTION
    ------------

    SW signing environment is used for secure signing of SW components and
    creation of mandatory flash images for the product from security point 
    of view. In addition, environment provides means to create certificates
    separately according to customer signing needs. 
    
    Included tools:
        SISS                - Tool for secure signing (can be used manually)
        FLIC                - Tool for flash image creation (can be used manually)
        make_ext4fs         - Tool for security file system creation 

    PLATFORM REQUIREMENTS
    ---------------------
    
    Toolset can be used in Linux and Windows (Cygwin) host environments
    with following requirements.

    The signing environment contains bash scripts which require that following
    standard Unix tools are in PATH:

        date
        grep
        perl
        sed
        uname

    All relatively recent Linux installations typically contain
    necessary tools. If Cygwin is preferred, availability of the tools
    listed above should be checked.

        
    USAGE
    -----

    Configuration:
        - If necessary, update configuration files in product ini-directory
    Input:
        - Populate product input-directories with required sub-images
    Execution:
        > ./image_signer.sh -product <product> -server <address:port> -productpath <path>
    Output:
        - Get resulting flash images from product output-directory
        - Get separate certificates from product output/certs-directory

    Additional information:
        Full command syntax:
        > ./image_signer.sh -help


    GENERAL NOTES
    -------------

    Single product configuration contains a set of files which are used to define parameters
    for environment, signing or flash image creation. Check samples for further details.

    Product-specific data has three main parts:
        - Product configuration files: Single product configuration contains a set of files which 
          are used to define parameters for environment, signing or flash image creation.
        - Input sub-images to construct necessary boot images
        - Output images and customer certificates

    Signing environment requires fixed directory structure for product-specific data.
    Location of the product data should be outside environment and has to be configured
    via command line switch '-productpath'. Check samples for further details
         


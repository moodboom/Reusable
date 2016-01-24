//
// File: ami_stream_single.C (formerly ami_single.C)
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 8/24/93
//

#include <versions.H>
VERSION(ami_single_C,"$Id: ami_stream_single.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include "lib_config.H"

// Don't bother defining any BTE implementation, since this is library
// code for the AMI.
#define BTE_STREAM_IMP_USER_DEFINED
#define BTE_STREAM BTE_stream_base
#define AMI_STREAM_IMP_SINGLE
#include <ami_stream.H>

// The default device description for AMI single streams.
AMI_device AMI_stream_single_base::default_device;

// The device index of the most recently created stream.
unsigned int AMI_stream_single_base::device_index;

// Initializer

unsigned int AMI_stream_single_base_device_initializer::count;

AMI_stream_single_base_device_initializer::
    AMI_stream_single_base_device_initializer(void)
{
    AMI_err ae;
        
    if (!count++) {
        // Try to initialize from the environment.
        ae = AMI_stream_single_base::
            default_device.read_environment(AMI_SINGLE_DEVICE_ENV);

        if (ae == AMI_ERROR_NO_ERROR) {        
            return;
        }

        // Try to initialize from TMP_DIR
        ae = AMI_stream_single_base::
            default_device.read_environment(TMPDIR_ENV);

        if (ae == AMI_ERROR_NO_ERROR) {
            return;
        }

        // Try to initialize to a default path
        ae = AMI_stream_single_base::
            default_device.set_to_path(TMP_DIR "|" TMP_DIR);

        if (ae != AMI_ERROR_NO_ERROR) {
            LOG_WARNING_ID("Unable to initialize the default device description for AMI single streams.");
        }

        LOG_DEBUG_ID("Default device description for AMI single streams:");
        LOG_DEBUG_ID(AMI_stream_single_base::default_device);

        // Set the last device index used to the last device index, so
        // that the first stream will wrap around to go on device 0.
        AMI_stream_single_base::device_index =
            AMI_stream_single_base::default_device.arity() - 1;
    }
}

AMI_stream_single_base_device_initializer::
    ~AMI_stream_single_base_device_initializer(void)
{
}

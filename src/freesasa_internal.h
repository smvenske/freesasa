#ifndef FREESASA_INTERNAL_H
#define FREESASA_INTERNAL_H

#include <stdio.h>
#include "freesasa.h"
#include "coord.h"

//! The name of the library, to be used in error messages and logging
extern const char *freesasa_name;

//! Classifier that classifies each atom according to residue
extern const freesasa_classifier freesasa_residue_classifier;

//! A ::freesasa_subarea with `name == NULL` and all values 0
extern const freesasa_subarea freesasa_subarea_null;

//! Shortcut for memory error generation
#define mem_fail() freesasa_mem_fail(__func__,__FILE__,__LINE__) 

//! Shortcut for error message with position information
#define fail_msg(msg) freesasa_fail_wloc(__func__,__FILE__,__LINE__,msg)


/**
    Calculate SASA using S&R algorithm.
    
    @param sasa The results are written to this array, the user has to
    make sure it is large enough.
    @param c Coordinates of the object to calculate SASA for.
    @param radii Array of radii for each sphere.
    @param param Parameters specifying resolution, probe radius and
    number of threads. If NULL :.freesasa_default_parameters is used.
    @return ::FREESASA_SUCCESS on success, ::FREESASA_WARN if multiple
    threads are requested when compiled in single-threaded mode (with
    error message). ::FREESASA_FAIL if memory allocation failure.
*/
int
freesasa_shrake_rupley(double *sasa,
                       const coord_t *c,
                       const double *radii,
                       const freesasa_parameters *param);

/**
    Calculate SASA using L&R algorithm.

    Solvent accessible surface area for each atom is written to the
    array 'sasa'. The user is responsible for making sure this has the
    right size. The argument grid sets the distance between grid
    points in Å. Returns FREESASA_SUCCESS on success, FREESASA_WARN if
    multiple threads are requested when compiled in single-threaded
    mode (with error message). 
    
    @param sasa The results are written to this array, the user has to
    make sure it is large enough.
    @param c Coordinates of the object to calculate SASA for.
    @param radii Array of radii for each sphere.
    @param param Parameters specifying resolution, probe radius and
    number of threads. If NULL :.freesasa_default_parameters is used.
    @return ::FREESASA_SUCCESS on success, ::FREESASA_WARN if
    multiple threads are requested when compiled in single-threaded
    mode (with error message). ::FREESASA_FAIL if memory allocation 
    failure.
*/
int freesasa_lee_richards(double* sasa,
                          const coord_t *c,
                          const double *radii,
                          const freesasa_parameters *param);


/**
    Get coordinates.
    
    @param s A structure.
    @return The coordinates of the structure as a ::coord_t struct.
 */
const coord_t *
freesasa_structure_xyz(const freesasa_structure *s);

/**
    Get a string describing an atom. 
    Format: "A    1 ALA  CA " 
    (chain label, residue number, residue type, atom name)

    @param s A structure.
    @param i Atom index
    @return Descriptor string. 
 */
const char*
freesasa_structure_atom_descriptor(const freesasa_structure *s,
                                   int i);

/**
    Get the index of a chain.

    @param s A structure.
    @param chain The chain label.
    @return The index of `chain` in the structure. ::FREESASA_FAIL 
    if `chain` not found.
    
 */
int
freesasa_structure_chain_index(const freesasa_structure *s,
                               char chain);

/**
    Get a string describing a residue.
    Format: "A    1 ALA" (chain label, residue number, residue type)
    
    @param s A structure.
    @param r_i atom index
    @return Descriptor string
 */
const char*
freesasa_structure_residue_descriptor(const freesasa_structure *s,
                                      int r_i);


/**
    Returns the SASA for a given residue

    @param r The SASA results
    @param s The structure
    @param r_i Index of residue
    @return The SASA of the residue
 */
double
freesasa_single_residue_sasa(const freesasa_result *r,
                             const freesasa_structure *s, 
                             int r_i);

/**
    Extract area to provided ::freesasa_subarea object

    Main-chain / side-chain atoms are defined by
    ::freesasa_backbone_classifier.

    @param area Area will be stored here
    @param structure Structure to use for classification
    @param result The areas to use
    @param polar_classifier Classifier to use to determine if 
      the atom is polar or not
    @param atom_index Index of atom in question  
 */
void
freesasa_atom_subarea(freesasa_subarea *area,
                      const freesasa_structure *structure,
                      const freesasa_result *result,
                      const freesasa_classifier *polar_classifier,
                      int atom_index);

/**
    Adds all members of term to corresponding members of sum

    @param sum Object to add to
    @param term Object to add
 */
void
freesasa_add_subarea(freesasa_subarea *sum,
                     const freesasa_subarea *term);
/**
    Calculate relative SASA values for a residue

    If the array `ref_values` does not have an entry that has the same
    `name` as `abs`, `rel->name` will be `NULL`.

    @param rel Store results here, will have same name as `abs`
    @param abs Absolute SASA for residue
    @param ref_values Array of reference values, will be iterated through
      until `ref_values[i].name == NULL`.
    @return ::FREESASA_SUCCESS. ::FREESASA_WARN if the name of the
       residue in `abs` not found in `ref_values`.
 */
int
freesasa_residue_rel_subarea(freesasa_subarea *rel,
                             const freesasa_subarea *abs,
                             const freesasa_classifier *classifier);
/**
    Calculates the absolute and relative SASA values for a given residue

    If `reference->max == NULL` then `rel` will be all zero and
    `rel->name == NULL`. This behavior allows the user to calculate
    absolute SASA values also when there's no reference to calculate
    relative values.

    @param abs Where to write absolute SASA
    @param rel Where to write relative SASA
    @param residue_index Index of residue
    @param structure The structure
    @param result The SASA values
    @param reference Reference values and classifiers
    @return ::FREESASA_SUCCESS. ::FREEESASA_FAIL if inconsistencies in structure.
 */
int
freesasa_rsa_val(freesasa_subarea *abs,
                 freesasa_subarea *rel,
                 int residue_index,
                 const freesasa_structure *structure,
                 const freesasa_result *result,
                 const freesasa_classifier *classifier);

/**
    Is an atom a backbone atom
   
    @param atom_name Name of atom
    @return 1 if the atom_name equals CA, N, O or C after whitespace
    is trimmed, 0 else. (i.e. does not check if it is an actual atom)
 */
int
freesasa_atom_is_backbone(const char *atom_name);

/**
    The maximum areas of a residue, for RSA calculation.

    Will only work where classifier->config points to a struct of type
    classifier_config.

    @param res_name Name of residue
    @param classifier The classifier to use
    @return The maximum areas (total, polar, etc). Will return an
      object where the name member is NULL if the residue does not
      have any maximums associated. This is not unusual, classifiers
      may have max values for amino acids but not nucleic acids for
      example.
 */
const freesasa_subarea *
freesasa_residue_max_area(const char *res_name, 
                          const freesasa_classifier *classifier);

/**
    The name of a classifier.

    Will only work where classifier->config points to a struct of type
    classifier_config.

    @param Classifier The classifier
    @return The name. NULL if not defined.
 */
const char *
freesasa_classifier_name(const freesasa_classifier *classifier);

/**
    Holds range in a file, to be initalized with ftell() and used
    with fseek().
 */
struct file_range {
    long begin; //!< Position of beginning of range
    long end; //!< Position of end of range
};

/**
    For convenience, get a file range that covers a whole file.

    @param file The file to study
    @return the ::file_range.
 */
struct file_range
freesasa_whole_file(FILE* file);

/**
    Print failure message using format string and arguments.

    @param format Format string
    @return ::FREESASA_FAIL
*/
int 
freesasa_fail(const char *format,...);

/**
    Print warning message using format string and arguments.

    @param format Format string
    @return ::FREESASA_WARN
 */
int
freesasa_warn(const char *format,...);

/**
    Print warning message using function, file and line-number. 

    Usually used via the macro mem_fail().

    @param func Name of the function that failed
    @param file The file where the function is.
    @param line Line number for the error.
    @return ::FREESASA_FAIL
 */
int
freesasa_mem_fail(const char* func,
                  const char* file,
                  int line);

/**
    Returns string explaining return values of pthread_create() and
    pthread_join().

    @param error_code The error code
    @return A string describing the error code.
 */
const char*
freesasa_thread_error(int error_code);

/**
    Prints fail message with function name, file name, and line number.

    Mainly intended to be used by the macros fail_msg() and mem_fail().

    @param func The function name
    @param file The file name
    @param line The line number
    @param msg The error message
    @return ::FREESASA_FAIL
 */
int
freesasa_fail_wloc(const char* func,
                   const char* file,
                   int line,
                   const char *msg);


#endif /* FREESASA_INTERNAL_H */

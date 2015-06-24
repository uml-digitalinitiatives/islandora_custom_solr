# Islandora Custom Solr

### SUMMARY
***
Replace Sparql queries with Solr queries where possible for speed improvements.

### REQUIREMENTS
***
Dependent on:
* islandora
* islandora\_solr

These modules are available for speed improvements, if you don't have one installed it's enhancements cannot be enabled.

Affects:
* islandora\_newspaper (for newspaper improvements)
* islandora\_book (for book improvements)
* islandora\_compound\_object (for compound improvements)


### INSTALLATION
***

Install as any other Drupal module.

### CONFIGURATION
***

Enable/Disable the improvements you wish (assuming you have the modules installed) at /admin/islandora/tools/custom_solr

Configure the correct Solr fields to use.

**Note**: This module disrupts the normal processing of content models from islandora\_newspaper, islandora\_book and islandora\_compound solution packs.

This means any theme templates or theme preprocess functions may need to be updated.

Please test before placing into production.

<?php

/**
 * @file dkan_harvest_dcatap.migrate.inc
 *
 * Migration File for DKAN Harvest of CKAN file from
 *    /api/3/action/current_package_list_with_resources .
 */

/**
 * Class HarvestMigrationDCATAP.
 *
 * Harvest the DCAT-AP schema.
 * @see https://www.w3.org/TR/vocab-dcat/
 * @see https://joinup.ec.europa.eu/asset/dcat_application_profile/description
 */
class HarvestMigrationDCATAP extends HarvestMigration {

  public $itemUrl;

  /**
   * Constructor.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->itemUrl = drupal_realpath($this->dkanHarvestSource->getCacheDir()) .
            '/:id';

    $this->source = new HarvestMigrateSourceList(
            new HarvestList($this->dkanHarvestSource->getCacheDir()),
            new MigrateItemJSON($this->itemUrl),
            $this->getCkanDatasetFields(),
            $this->sourceListOptions
    );
  }

  /**
   * {@inheritdoc}
   */
  public function setFieldMappings($version = '') {
    parent::setFieldMappings();

    $this->addFieldMapping('field_tags', 'keyword');
    $this->addFieldMapping('field_tags:create_term')
      ->defaultValue(TRUE);

    $this->addFieldMapping('changed', 'modified');
    $this->addFieldMapping('created', 'issued');
    $this->addFieldMapping('field_public_access_level', 'accessLevel');
    
    // Calculated field
    $this->addFieldMapping('field_contact_name', '_mbox_name');
    $this->addFieldMapping('field_contact_email', '_mbox');
    
    $this->addFieldMapping('uuid', 'identifier');
    $this->addFieldMapping('field_license', 'license');
    $this->addFieldMapping('field_spatial_geographical_cover', 'spatial');
    $this->addFieldMapping('field_temporal_coverage', 'temporalBegin');
    $this->addFieldMapping('field_temporal_coverage:to', 'temporalEnd');
    $this->addFieldMapping('field_frequency', 'accrualPeriodicity');
    $this->addFieldMapping('field_data_dictionary', 'describedBy');
    $this->addFieldMapping('field_landing_page', 'landingPage');
    $this->addFieldMapping('field_rights', 'rights');
    $this->addFieldMapping('field_conforms_to', 'conformsTo');
    $this->addFieldMapping('field_data_dictionary_type', 'describedByType');
    $this->addFieldMapping('field_is_part_of', 'isPartOf');
    $this->addFieldMapping('field_language', 'language');
    $this->addFieldMapping('field_pod_theme', 'theme');
    $this->addFieldMapping('field_pod_theme:create_term')
      ->defaultValue(TRUE);
    
    $this->addFieldMapping('field_related_content', 'references');
    
    // Use the publisher.name for author
    $this->addFieldMapping('field_author', '_publisher');
    
    // dkan_dataset_metadata_source.
    if (module_exists('dkan_dataset_metadata_source')) {
      $this->addFieldMapping('field_metadata_sources', 'metadata_sources');
    }
  }

  /**
   * Implements prepareRow.
   */
  public function prepareRow($row) {
    parent::prepareRow($row);
    
    if(isset($row->contactPoint->hasEmail)){
      $row->_mbox = $row->contactPoint->hasEmail;
    }
    
    if(isset($row->contactPoint->fn) && $row->contactPoint->fn!='<Nobody>'){
      $row->_mbox_name = $row->contactPoint->fn;
    }
    
    if(isset($row->publisher->name) && $row->publisher->name != '') {
      $row->_publisher = $row->publisher->name;
    }
  }

  /**
   * Implements prepare().
   */
  public function prepare($dataset_prepare, $row) {
    parent::prepare($dataset_prepare, $row);

    // Prepare group data.
    $row->group = $this->prepareRowGroup($row->publisher);

    // Prepare resources data.
    if (isset($row->distribution)) {
      $row->resources = $this->prepareRowResources($row->distribution);
    }
  }

  /**
   * Prepare list of resources that need to be imported based on the row data.
   *
   * @param object $resources_row_data
   *   Resources data present on row.
   *
   * @return
   *   An array with all resources data ready to be imported or empty if
   *   no resources where found.
   */
  public function prepareRowResources($resources_row_data) {
    $resources = array();

    foreach ($resources_row_data as $resource_row_data) {
      $resource = $this->prepareResourceHelper(
        $resource_row_data->downloadURL, 
        $resource_row_data->format, 
        $resource_row_data->title, 
        NULL, 
        $resource_row_data->description
      );

      if ($resource) {
        $resources[] = $resource;
      }
    }

    return $resources;
  }

  /**
   * Prepare group that needs to be imported based on the row data.
   *
   * @param object $group_row_data
   *    Group data present on row.
   *
   * @return
   *    An object value with the group data needed to import the group.
   */
  public function prepareRowGroup($group_row_data) {

    $group = new stdClass();
    $group->name = $group_row_data->name;

    return $group;
  }

}

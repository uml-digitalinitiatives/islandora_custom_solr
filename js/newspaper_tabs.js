(function ($) {
  Drupal.behaviors.islandora_custom_solr = {
    attach: function(context, settings) {
      jQuery('#islandora_custom_solr_ajax_nav .vertical-tab-button a').once('islandora_custom_solr_ajax_nav', function() {
        // Use context to ensure the link is only ever activated if it's regenerated.
        var $mySpecialLink = $(this, context);
        var year = jQuery(this).children('strong').text();
        var pid = Drupal.settings.islandora_custom_solr.newspaper_issue_pid;
        if (pid.length > 0 && year != Drupal.settings.islandora_custom_solr.first_year) {
          jQuery(this).attr('href', Drupal.settings.islandora_custom_solr.base_url + '/islandora_custom_solr/' + pid + '/newspaper_year/' + year + '/nojs').addClass('use-ajax');
          Drupal.ajax['fieldset-content-' + year] = new Drupal.ajax('#fieldset-content-' + year, $mySpecialLink, {
            url: $mySpecialLink.attr('href'),
            settings: {},
            event: 'click tap'
          });
        }
      });
    }
  }
  Drupal.islandora_custom_solr.islandora_custom_solr_finish_ajax = function() {
    jQuery(".fieldset-legend", this).wrapInner('<a href="#" class="fieldset-title"></a>').click(function(){
      jQuery(this).parents('legend').siblings(".fieldset-wrapper").toggle();
    });
  }

})(jQuery);

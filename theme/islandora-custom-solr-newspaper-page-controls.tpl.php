<?php
/**
 * @file
 * Displays the newspaper page controls.
 */
?>
<div class="islandora-newspaper-controls islandora-custom-solr">
  <?php print theme('item_list', array('items' => $controls, 'attributes' => array('class' => array('items', 'inline')))); ?>
</div>

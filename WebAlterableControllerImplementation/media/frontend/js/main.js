var UpdatingCollectionView = Backbone.View.extend({
  initialize : function(options) {
    _(this).bindAll('add', 'remove');
 
    if (!options.childViewConstructor) throw "no child view constructor provided";
    if (!options.childViewTagName) throw "no child view tag name provided";
 
    this._childViewConstructor = options.childViewConstructor;
    this._childViewTagName = options.childViewTagName;
 
    this._childViews = [];
 
    this.collection.each(this.add);
 
    this.collection.bind('add', this.add);
    this.collection.bind('remove', this.remove);
  },
 
  add : function(model) {
    var childView = new this._childViewConstructor({
      tagName : this._childViewTagName,
      model : model
    });
 
    this._childViews.push(childView);
 
    if (this._rendered) {
      $(this.el).append(childView.render().el);
    }
  },
 
  remove : function(model) {
    var viewToRemove = _(this._childViews).select(function(cv) { return cv.model === model; })[0];
    this._childViews = _(this._childViews).without(viewToRemove);
 
    if (this._rendered) $(viewToRemove.el).remove();
  },
 
  render : function() {
    var that = this;
    this._rendered = true;
 
    $(this.el).empty();
 
    _(this._childViews).each(function(childView) {
      $(that.el).append(childView.render().el);
    });
 
    return this;
  }
});



$(function() {

    $.xhrPool = [];
    $.xhrPool.abortAll = function() {
        $(this).each(function(idx, jqXHR) {
            jqXHR.abort();
        });
        $.xhrPool.length = 0;
    };

    $.ajaxSetup({
        beforeSend: function(jqXHR) {
            $.xhrPool.push(jqXHR);
        },
        complete: function(jqXHR) {
            var index = $.xhrPool.indexOf(jqXHR);
            if (index > -1) {
                $.xhrPool.splice(index, 1);
            }
        }
    });


    window.ValueUpdater = function() {
        return {
            _values: {},
            continuous: false,
            alreadyChecking: false,

            reset: function() {
                this._values = {};
            },

            add: function(in_value) {
                console.log(this);
                this._values[in_value.id] = in_value;
                if (in_value.parent.attributes.representation != 'text')
                    in_value.on("change", this.startCheck, this);
                this.startCheck();
            },

            startCheck: function(forceCheck) {

                if((this.alreadyChecking === false) || (forceCheck === true)) {
                    // console.log("start checking ...");
                    this.alreadyChecking = true;
                    var that = this;
                    _.delay(function() {
                        that.check();
                    }, 100);
                }
            },

            check: function() {
                // do the work
                // console.log("do checking ...");

                this.performRequests();

                // falls wir nicht loopen, alreadyCheck resetten, damit der nächste startCheck wieder ausgelöst wird
                if (this.continuous === false)
                    this.alreadyChecking = false;
                else
                    this.startCheck(true);
            },

            performRequests: function() {

                $.xhrPool.abortAll();

                /*
                // jedes value separat queryen
                _.each(this._values, function(value) {
                    $.ajax("/get_value"+value.id, {
                        success: function(data, textStatus,jqXHR) {
                            data = JSON.parse(data);
                            value.set("value", data.result);
                        }
                    });
                });
                */

                // alle values auf einmal queryen
                var that = this;
                $.ajax("/get_value_list", {
                    success: function(data, textStatus,jqXHR) {
                            data = JSON.parse(data);
                            _.each(data.result, function(value, key, list) {
                                if(that._values[key] !== undefined) {
                                    that._values[key].set("value", value);
                                }
                            });
                        }
                });

            }
        };
    };

    window.AlterableValue = Backbone.Model.extend( {

        defaults: function() {
            return { key: "unknown", value: 0};
        },

        initialize: function() {
            // console.log("AlterableValue.initiazlie");
            this.id = this.attributes.key;
            this.set("html_id", this.attributes.key.replace(/\//g, "-").substr(1));
        },

        getRepresentationType: function() {
            return this.parent.get("representation");
        }
    });

    window.AlterableValueList = Backbone.Collection.extend({
        model: AlterableValue,

        initialize: function() {
            // console.log("AlterableValueList.initiazlie");
        }
    });

    window.Alterable = Backbone.Model.extend( {

        defaults: function() {
            return { name: "unknown", values: null };
        },

        initialize: function() {
            //console.log("Alterable.initiazlie");
            
            this.values = new AlterableValueList();
            
            var that = this;
            this.values.on("add", function(value) {
                value.parent = that;
                window.app.updater.add(value);
            });

            this.values.add(this.attributes.values);
        }
    });

    window.AlterableList = Backbone.Collection.extend({

        model: Alterable,

        initialize: function() {
            //console.log("AlterableList.initiazlie");
        }

    });

    window.Group = Backbone.Model.extend( {

        alterables: null,

        defaults: function() {
            return { name: "unknown"};
        },

        initialize: function() {
            //console.log("group.initiazlie");
            
            this.alterables = new AlterableList();
            this.alterables.add(this.attributes.values);
        }
    });


    window.Groups = Backbone.Collection.extend( {
        model: Group,
        url: '/get_structure'
    });

    window.UpdatingValueView = Backbone.View.extend( {
        tagName: 'li',
        classname: 'value',
        template: null,

        initialize: function() {
            var that = this;

            this.update = _.bind(this.update, this);
            this.model.bind("change:value", function() { that.update(); });

            var template_name = '#value-template-'+this.model.getRepresentationType();
            
            
            if ($(template_name).length === 0)
                template_name = '#value-template';

            this.template = _.template($(template_name).html());
        },

        events: {
            "click .button button": "handleButton",
            "change .value input[type=text]": "handleValueChange",
            "change .value input[type=range]": "handleValueChange",
            "change .switch input[type=checkbox]": "handleSwitchChange"
        },

        render: function() {
            $(this.el).html(this.template({value: this.model.toJSON(), alterable: this.model.parent.toJSON()}));
            return this;
        },

        update: function() {
            switch(this.model.parent.attributes.representation) {
                
                case 'text':
                    this.$el.find("span.value").html(this.model.get("value"));
                    break;
                
                case 'switch':
                    var elem = this.$el.find("input[type=checkbox]");
                    if (this.model.get("value") == 1)
                        elem.attr('checked',  'checked');
                    else
                        elem.removeAttr('checked');
                    break;
                
                case 'value':
                    this.$el.find("input[type=text]").val(this.model.get("value"));
                    this.$el.find("input[type=range]").val(this.model.get("value"));
                    break;
            }
        },

        handleButton: function() {
            console.log("handleButton "+this.model.id);
            this.model.set("value", 1);
            this.model.save();
        },

        handleValueChange: function(elem) {
            var input_elem = elem.target;
            console.log("handleValueChange: "+$(input_elem).val());
            this.model.set("value", $(input_elem).val());
            this.model.save();
        },

        handleSwitchChange: function(elem) {
            var input_elem = elem.target;
            var new_val = ($(input_elem).attr('checked') === 'checked') ? 1 : 0;
            console.log("handleValueChange: "+new_val);
            this.model.set("value", new_val);
            this.model.save();
        }
    });


    window.UpdatingAlterableView = Backbone.View.extend( {
        tagName : "li",
        className: 'alterable',
        template: _.template($('#alterable-template').html()),

        initialize: function() {
            this._valueListView = new UpdatingCollectionView({
                collection           : this.model.values,
                childViewConstructor : UpdatingValueView,
                childViewTagName     : 'li'
            });
        },

        render: function() {
            $(this.el).html(this.template(this.model.toJSON()));
            this._valueListView.el = this.$('.values');
            this._valueListView.render();

            return this;
        }

    });

    window.GroupView = Backbone.View.extend( {

        template: _.template($('#group-template').html()),

        initialize: function() {
            
            this._alterableListView = new UpdatingCollectionView({
                collection           : this.model.alterables,
                childViewConstructor : UpdatingAlterableView,
                childViewTagName     : 'li'
            });
            
            this.model.bind('change', this.render, this);
            this.model.bind('remove', this.remove, this);
        },

        render: function() {

            $(this.el).html(this.template(this.model.toJSON()));
            this._alterableListView.el = this.$('.alterables');
            this._alterableListView.render();
            
            return this;
        },

        remove: function() {
            $(this.el).remove();
        }
    
    });

    window.AppView = Backbone.View.extend( {

        groups: null,
        updater: null,

        initialize: function() {

            this.updater = new ValueUpdater();

            this.groups = new Groups();

            this.groups.bind('add',   this.addOne, this);
            this.groups.bind('reset', this.addAll, this);
            this.groups.bind('all',   this.render, this);

            //this.refresh();
        },

        events: {
            "click #refresh":  "refresh",
            "click #dump" : "dump",
            "change #continuous-update" : "toggleUpdater"
        },

        addOne: function(group) {
            var view = new GroupView({model: group});
            $(this.el).children('.listing').append(view.render().el);
        },

        addAll: function() {
            $(this.el).children('.listing').html("");
            this.groups.each(this.addOne, this);
        },

        refresh: function() {
            console.log("refresh");
            this.groups.fetch();
            console.log(this.groups);
            this.render();
        },

        dump: function() {
            console.log(this);
        },

        toggleUpdater: function(elem) {
            var input_elem = elem.target;
            var toggled = ($(input_elem).attr('checked') === 'checked') ? true : false;
            console.log("toggle updater: "+toggled);
            this.updater.continuous = toggled;
            if (toggled)
                this.updater.startCheck();
        },

        render: function() {
            return this;
        }
    });

    Backbone.sync = function(method, model, options) {
        
        switch (method) {
            case "read":
                if(model.id) {
                } else {
                    $.ajax(model.url, {
                        success: function(data, textStatus,jqXHR) {
                            data = JSON.parse(data);
                            options.success(data.result.groups);
                        }
                    });
                }
                break;
            
            case "update":
                if(model.id) {
                    $.ajax('/set_value'+model.id+"="+model.get("value"), {
                        success: function(data, textStatus,jqXHR) {
                            options.success(data.result);
                        }
                    });
                }
                break;
        }
    };



    window.app = new AppView({el: $("#app-container")});
});
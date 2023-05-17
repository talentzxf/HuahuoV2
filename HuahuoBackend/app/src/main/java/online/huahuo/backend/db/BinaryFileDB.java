package online.huahuo.backend.db;

import com.fasterxml.jackson.annotation.JsonIgnore;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.*;
import java.util.Date;

@Entity
@Data
@NoArgsConstructor
@Table(name = "BINARYFILES", indexes = {
        @Index(columnList = "name"),
        @Index(columnList = "createdBy")
}, uniqueConstraints = {
        @UniqueConstraint(columnNames = {"createdBy", "name"}) // Can't create two same name files under the same creator.
})
public class BinaryFileDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    private String name;
    private String version;

    private String createdBy;
    private String description;

    @JsonIgnore
    @Column(unique = true)
    private String fullPath;

    @JsonIgnore
    private String coverPagePath;
    private String checksum;

    private Date createTime;
    private Date modifiedTime;

    @Column(nullable = false)
    private BinaryFileStatus status;

    private FileType fileType;


    public BinaryFileDB(String name, String version, String createdBy,
                        String fullPath, String checksum, String description, FileType fileType){
        this.name = name;
        this.version = version;
        this.fullPath = fullPath;
        this.checksum = checksum;
        this.createdBy = createdBy;
        this.createTime = new Date();
        this.modifiedTime = new Date();
        this.status = BinaryFileStatus.ACTIVE;
        this.description = description;
        this.fileType = fileType;
    }
}
